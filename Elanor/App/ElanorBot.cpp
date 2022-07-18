#include <Command/Command.hpp>
#include <Trigger/Trigger.hpp>
#include <Group/GroupList.hpp>
#include <Client/Client.hpp>
#include <Utils/Utils.hpp>
#include <State/AccessCtrlList.hpp>
#include <ThirdParty/log.h>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

#include <mirai.h>

#include "ElanorBot.hpp"

using namespace std;

namespace Bot
{

vector<pair<string, unique_ptr<GroupCommand::GroupCommandBase>>> RegisterCommands()
{
	vector<pair<string, unique_ptr<GroupCommand::GroupCommandBase>>> v;
	#define REGISTER(_class_) v.push_back(move(make_pair( string(_class_::_NAME_), move(make_unique<_class_>()))));

	REGISTER(GroupCommand::WhiteList)
	REGISTER(GroupCommand::BlackList)
	REGISTER(GroupCommand::CommandAuth)
	REGISTER(GroupCommand::SetTrigger)

	REGISTER(GroupCommand::RollDice)
	REGISTER(GroupCommand::Repeat)
	REGISTER(GroupCommand::Recall)
	REGISTER(GroupCommand::AtBot)
	REGISTER(GroupCommand::Bililive)
	REGISTER(GroupCommand::Answer)

	REGISTER(GroupCommand::Petpet)
	REGISTER(GroupCommand::Choyen)
	REGISTER(GroupCommand::ImageSearch)
	REGISTER(GroupCommand::Pixiv)

	REGISTER(GroupCommand::pjskUpdate)
	REGISTER(GroupCommand::pjskSongGuess)
	REGISTER(GroupCommand::pjskCoverGuess)
	REGISTER(GroupCommand::pjskChart)
	REGISTER(GroupCommand::pjskMusicInfo)

	#undef REGISTER

	sort(v.begin(), 
		v.end(), 
		[](const pair<string, unique_ptr<GroupCommand::GroupCommandBase>>& a, const pair<string, unique_ptr<GroupCommand::GroupCommandBase>>& b)
		{
			return (a.second)->Priority() > (b.second)->Priority();
		});

	return v;
}

vector<pair<string, unique_ptr<Trigger::TriggerBase>>> RegisterTriggers()
{
	vector<pair<string, unique_ptr<Trigger::TriggerBase>>> v;
	#define REGISTER(_class_) v.push_back(move(make_pair( string(_class_::_NAME_), move(make_unique<_class_>()))));

	REGISTER(Trigger::BililiveTrigger)
	REGISTER(Trigger::MorningTrigger)

	#undef REGISTER
	return v;
}

ElanorBot::ElanorBot(Cyan::QQ_t owner_id) :
	is_running(false), suid(owner_id), group_commands(RegisterCommands()), triggers(RegisterTriggers())
{
	vector<pair<string, int>> command_list;
	vector<pair<string, bool>> trigger_list;
	command_list.reserve(this->group_commands.size());
	trigger_list.reserve(this->triggers.size());

	for (const auto& p : this->group_commands)
	{
		command_list.emplace_back(p.first, p.second->Permission());
	}
	for (const auto& p : this->triggers)
	{
		trigger_list.emplace_back(p.first, p.second->TriggerOnStart());
	}

	this->groups = make_shared<GroupList>(owner_id, command_list, trigger_list);
	for (const auto& p : this->triggers)
	{
		p.second->SetGroups(this->groups);
	}
}

void ElanorBot::NudgeEventHandler(Cyan::NudgeEvent& e)
{
	{
		lock_guard<mutex> lk(this->mtx);
		if (!this->is_running)	
			return;
	}

	try
	{
		Client& client = Client::GetClient();
		// 如果别人戳机器人，那么就让机器人戳回去
		if (e.Target != client.Call(&Cyan::MiraiBot::GetBotQQ))
			return;
		if (e.FromId == client.Call(&Cyan::MiraiBot::GetBotQQ))
			return;

		if (e.FromKind == Cyan::NudgeEvent::SubjectKind::Group)
		{
			string sender = client.Call(&Cyan::MiraiBot::GetGroupMemberInfo, (Cyan::GID_t)e.RawSubjectId, e.FromId).MemberName;
			string group_name = client.Call(&Cyan::MiraiBot::GetGroupConfig, (Cyan::GID_t)e.RawSubjectId).Name;
			logging::INFO("有人戳bot <OnNudgeEvent>\t<- [" + sender + "(" + to_string(e.FromId.ToInt64()) + "), " + group_name + "(" + to_string(((Cyan::GID_t)e.RawSubjectId).ToInt64()) + ")]");
			
			unique_lock<mutex> lock(this->nudge_mtx, try_to_lock);
			if (!lock)
			{
				logging::INFO("冷却中 <OnNudgeEvent>");
				return;
			}
			
			uniform_int_distribution<int> rng05(0, 5);
			int i = rng05(Utils::rng_engine);
			if (i)
			{
				this_thread::sleep_for(chrono::seconds(1));
				client.Call([](const unique_ptr<Cyan::MiraiBot>& bot, Cyan::QQ_t qq, Cyan::GID_t gid) { bot->SendNudge(qq, gid); }, 
					e.FromId, (Cyan::GID_t)e.RawSubjectId);	// ugly
				logging::INFO("戳回去了 <OnNudgeEvent>\t-> [" + sender + "(" + to_string(e.FromId.ToInt64()) + "), " + group_name + "(" + to_string(((Cyan::GID_t)e.RawSubjectId).ToInt64()) + ")]");
			}
			else
			{
				client.Send((Cyan::GID_t)e.RawSubjectId, Cyan::MessageChain().At(e.FromId).Plain(" 戳你吗"));
				logging::INFO("骂回去了 <OnNudgeEvent>\t-> [" + sender + "(" + to_string(e.FromId.ToInt64()) + "), " + group_name + "(" + to_string(((Cyan::GID_t)e.RawSubjectId).ToInt64()) + ")]");
			}
			this_thread::sleep_for(chrono::seconds(1));
		}
	}
	catch (Cyan::MiraiApiHttpException &e)
	{
		logging::WARN("<" + to_string(e.Code) + "> " + e.Message);
	}
}

void ElanorBot::GroupMessageEventHandler(Cyan::GroupMessage& gm)
{
	{
		lock_guard<mutex> lk(this->mtx);
		if (!this->is_running)	
			return;
	}

	Client& client = Client::GetClient();
	if (gm.Sender.QQ == client.Call(&Cyan::MiraiBot::GetBotQQ))
		return;
	
	Group& group = this->groups->GetGroup(gm.Sender.Group.GID);
	
	auto access_list = group.GetState<State::AccessCtrlList>();

	if (access_list->IsBlackList(gm.Sender.QQ))
		return;

	int auth = 0;
	if (gm.Sender.QQ == this->suid)
		auth = 100;
	else if (access_list->IsWhiteList(gm.Sender.QQ))
		auth = 50;
	else
	{
		switch (gm.Sender.Permission)
		{
		case Cyan::GroupPermission::Member:
			auth = 0; break;
		case Cyan::GroupPermission::Administrator:
			auth = 10; break;
		case Cyan::GroupPermission::Owner:
			auth = 20; break; 
		}
	}

	int priority = -1;
	for (const auto& p : this->group_commands)
	{
		if ((p.second)->Priority() < priority)
			break;
		vector<string> token;
		if ((p.second)->Parse(gm.MessageChain, token))
		{
			if (auth >= p.second->Permission())
			{
				try
				{
					(p.second)->Execute(gm, group, token);
				}
				catch (const exception& e)
				{
					logging::ERROR(e.what());
				}
			}
			else
			{
				logging::INFO("权限不足 <OnGroupMessage>: " + ((token.size())? token[0] : string() + Utils::GetDescription(gm, false)));
				client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("权限不足捏~"));
			}
			priority = (p.second)->Priority();
		}
	}
}

void ElanorBot::LostConnectionHandler(Cyan::LostConnection& e)
{
	logging::WARN("<" + to_string(e.Code) + "> " + e.ErrorMessage);
	this->is_running = false;
	int sleep = 2;
	while (true)
	{
		try
		{
			logging::INFO("尝试连接 mirai-api-http...");
			Client::GetClient().Reconnect();
			logging::INFO("与 mirai-api-http 重新建立连接!");
			break;
		}
		catch (const std::exception& ex)
		{
			logging::WARN(ex.what());
		}
		this_thread::sleep_for(chrono::seconds(sleep));
		sleep = (sleep > 30)? 60 : sleep * 2;
	}
	this->is_running = true;
}

void ElanorBot::EventParsingErrorHandler(Cyan::EventParsingError& e)
{
	try
	{
		e.Rethrow();
	}
	catch (const std::exception &ex)
	{
		logging::WARN("解析事件时出现错误: " + string(ex.what()));
	}
}

}