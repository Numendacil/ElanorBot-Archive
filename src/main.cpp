// 注意: 本项目的所有源文件都必须是 UTF-8 编码

#include <unordered_map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <filesystem>
#include <mirai.h>
#include "Utils.hpp"
#include "ElanorBot.hpp"
#include "Factory.hpp"
#include "MessageQueue.hpp"
#include "third-party/log.h"
#include "Command/GroupCommandBase.hpp"


using namespace std;
using namespace Cyan;

int main()
{
	// Call this before everything else
	Utils::Init();

	const QQ_t Owner = 1942036996_qq;

	unordered_map<GID_t, shared_ptr<ElanorBot>> Bots;
	shared_ptr<MiraiBot> client = make_shared<MiraiBot>();
	SessionOptions opts = SessionOptions::FromJsonFile("./config.json");

	vector<string> list = Factory<GroupCommandBase>::GetKeyList();
	vector<pair<string, unique_ptr<GroupCommandBase>>> CommandList;
	for (const auto& s : list)
		CommandList.emplace_back(s, Factory<GroupCommandBase>::Make(s));
	sort(CommandList.begin(), 
		CommandList.end(), 
		[](const pair<string, unique_ptr<GroupCommandBase>>& a, const pair<string, unique_ptr<GroupCommandBase>>& b)
		{
			return (a.second)->Priority() > (b.second)->Priority();
		});

	client->On<NudgeEvent>([client](NudgeEvent e)
	{
		try
		{
			static mutex mtx;
			// 如果别人戳机器人，那么就让机器人戳回去
			if (e.Target != client->GetBotQQ())
				return;
			if (e.FromId == client->GetBotQQ())
				return;

			if (e.FromKind == NudgeEvent::SubjectKind::Group)
			{
				string sender = client->GetGroupMemberInfo((GID_t)e.RawSubjectId, e.FromId).MemberName;
				string group_name = client->GetGroupConfig((GID_t)e.RawSubjectId).Name;
				logging::INFO("有人戳bot <OnNudgeEvent>\t<- [" + sender + "(" + to_string(e.FromId.ToInt64()) + "), " + group_name + "(" + to_string(((GID_t)e.RawSubjectId).ToInt64()) + ")]");
				
				unique_lock<mutex> lock(mtx, try_to_lock);
				if (!lock)
				{
					logging::INFO("冷却中 <OnNudgeEvent>");
					return;
				}
				
				uniform_int_distribution<int> rng05(0, 5);
				int i = rng05(Utils::rng_engine);
				if (i)
				{
					MiraiBot::SleepSeconds(1);
					client->SendNudge(e.FromId, (GID_t)e.RawSubjectId);
					logging::INFO("戳回去了 <OnNudgeEvent>\t-> [" + sender + "(" + to_string(e.FromId.ToInt64()) + "), " + group_name + "(" + to_string(((GID_t)e.RawSubjectId).ToInt64()) + ")]");
				}
				else
				{
					MessageQueue::GetInstance().Push((GID_t)e.RawSubjectId, MessageChain().At(e.FromId).Plain(" 戳你吗"));
					logging::INFO("骂回去了 <OnNudgeEvent>\t-> [" + sender + "(" + to_string(e.FromId.ToInt64()) + "), " + group_name + "(" + to_string(((GID_t)e.RawSubjectId).ToInt64()) + ")]");
				}
				MiraiBot::SleepSeconds(1);
			}
		}
		catch (MiraiApiHttpException &e)
		{
			logging::WARN("<" + to_string(e.Code) + "> " + e.Message);
		}
	});


	client->On<GroupMessage>([&, client , &CommandList = as_const(CommandList)](GroupMessage gm) 
	{
		if (gm.Sender.QQ == client->GetBotQQ())
			return;
		
		shared_ptr<ElanorBot> bot;
		static mutex mtx_bots;
		{
			lock_guard<mutex> lk(mtx_bots);
			if (!Bots[gm.Sender.Group.GID])
				Bots[gm.Sender.Group.GID] = make_shared<ElanorBot>(gm.Sender.Group.GID, Owner, client);
			bot = Bots[gm.Sender.Group.GID];
		}

		int priority = -1;
		for (const auto& p : CommandList)
		{
			if ((p.second)->Priority() < priority)
				break;
			vector<string> token;
			if ((p.second)->Parse(gm.MessageChain, token))
			{
				if (bot->CheckAuth(gm.Sender, p.first))
				{
					(p.second)->Execute(gm, bot, token);
					priority = (p.second)->Priority();
				}
				else
				{
					logging::INFO("权限不足 <OnGroupMessage>: " + ((token.size())? token[0] : string() + Utils::GetDescription(gm, false)));
					MessageQueue::GetInstance().Push(gm.Sender.Group.GID, MessageChain().Plain("权限不足捏~"));
				}
			}
		}
	});


	// 在失去与mah的连接后重连
	client->On<LostConnection>([client](LostConnection e)
	{
		MessageQueue::GetInstance().Pause();
		logging::WARN("<" + to_string(e.Code) + "> " + e.ErrorMessage);
		MiraiBot::SleepSeconds(20);
		int sleep = 2;
		while (true)
		{
			try
			{
				logging::INFO("尝试连接 mirai-api-http...");
				client->Reconnect();
				logging::INFO("与 mirai-api-http 重新建立连接!");
				MessageQueue::GetInstance().Resume();
				break;
			}
			catch (const std::exception& ex)
			{
				logging::WARN(ex.what());
			}
			MiraiBot::SleepSeconds(sleep);
			sleep = (sleep > 30)? 60 : sleep * 2;
		}
	});


	client->On<EventParsingError>([](EventParsingError e)
	{
		try
		{
			e.Rethrow();
		}
		catch (const std::exception& ex)
		{
			logging::WARN("解析事件时出现错误: " + string(ex.what()));
		}
	});



	while (true)
	{
		try
		{
			logging::INFO("尝试与 mirai-api-http 建立连接...");
			client->Connect(opts);
			break;
		}
		catch (const std::exception& ex)
		{
			logging::WARN(ex.what());
		}
		MiraiBot::SleepSeconds(3);
	}

	// 检查一下版本
	try
	{
		string mah_version = client->GetMiraiApiHttpVersion();
		string mc_version = client->GetMiraiCppVersion();
		logging::INFO("mirai-api-http 的版本: " + mah_version
			+ "; mirai-cpp 的版本: " + mc_version);
		if (mah_version != mc_version)
		{
			logging::WARN("Warning: 你的 mirai-api-http 插件的版本与 mirai-cpp 的版本不同，可能存在兼容性问题。");
		}
	}
	catch (const std::exception& ex)
	{
		logging::WARN(ex.what());
	}
	MessageQueue::GetInstance().Start(client);
	string path = "./bot";
	for (const auto & entry : filesystem::directory_iterator(path))
	{
		if (entry.is_regular_file())
		{
			try
			{
				GID_t gid = (GID_t)stol(entry.path().stem());
				Bots[gid] = make_shared<ElanorBot>(gid, Owner, client);
			}
			catch(const logic_error& e)
			{
				logging::WARN("Unexpected file found in ./bot directory: " + string(entry.path().filename()));
			}
		}
	}
	logging::INFO("Bot Working...");



	string cmd;
	while (cin >> cmd)
	{
		if (cmd == "exit")
		{
			MessageQueue::GetInstance().Stop();
			client->Disconnect();
			break;
		}
	}
	return 0;
}