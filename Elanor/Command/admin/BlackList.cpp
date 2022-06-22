#include <stdexcept>

#include <mirai/exceptions/exceptions.hpp>

#include <third-party/log.h>

#include <Command/admin/BlackList.hpp>

#include <Utils/Utils.hpp>

#include <app/ElanorBot.hpp>

using namespace std;
using namespace Cyan;

bool BlackList::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#black"))
	{
		Utils::ToLower(str);
		Utils::Tokenize(token, str);
		if (token[0] == "#black" || token[0] == "#黑名单" || token[0] == "#blacklist")
			return true;
	}
	return false;
}

bool BlackList::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	logging::INFO("Calling BlackList <BlackList>" + Utils::GetDescription(gm));
	string command = token[1];
	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档 <BlackList>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#blacklist {add/delete/exist} [QQ]...\n#blacklist {clear/clean/list}"));
		return true;
	}

	if (command == "clear")
	{
		bot->BlackListClear();
		logging::INFO("清除成功 <BlackList>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("黑名单归零了捏"));
		return true;
	}

	if (command == "clean")
	{
		auto list = bot->GetBlackList();
		for (const auto& id : list)
		{
			try
			{
				bot->client->GetGroupMemberInfo(gm.Sender.Group.GID, id);
			}
			catch (const MiraiApiHttpException& e)	// No such member
			{
				bot->BlackListDelete(id);
			}
		}
		logging::INFO("整理成功 <BlackList>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("黑名单打扫好了捏"));
		return true;
	}

	if (command == "list")
	{
		auto list = bot->GetBlackList();
		string msg = "本群黑名单:\n";
		for (const auto& id : list)
		{
			try
			{
				auto profile = bot->client->GetGroupMemberInfo(gm.Sender.Group.GID, id);
				msg += to_string(id.ToInt64()) + " (" + profile.MemberName + ")\n";
			}
			catch (const MiraiApiHttpException& e)	// No such member
			{
				msg += to_string(id.ToInt64()) + " (目前不在群内)\n";
			}
		}
		logging::INFO("输出名单 <BlackList>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain(msg));
		return true;
	}

	if (command == "exist" || command == "add" || command == "del" || command == "delete")
	{
		auto AtMsg = gm.MessageChain.GetAll<AtMessage>();
		if (token.size() + AtMsg.size() < 3)
		{
			logging::INFO("缺少参数[QQ] <BlackList>: " + command + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("缺少参数[QQ]，是被你吃了嘛"));
			return false;
		}

		vector<QQ_t> arr;
		int i;
		try
		{
			for (i = 2; i < token.size(); ++i)
				arr.push_back(QQ_t(stol(token[i])));
		}
		catch (const logic_error& e)
		{
			logging::INFO("无效参数[QQ] <BlackList>: " + token[i] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(token[i] + "是个锤子QQ号"));
			return false;
		}
		for (const auto& p : AtMsg)
			arr.push_back(p.Target());

		if (command == "exist")
		{
			string msg = "查询结果:\n";
			for (const auto& id : arr)
			{
				msg += to_string(id.ToInt64()) + ((bot->IsBlackList(id))? " 在黑名单中\n" : " 不在黑名单中\n");
			}
			logging::INFO("查询成功 <BlackList>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(msg));
			return true;
		}

		if (command == "add")
		{
			for (const auto& id : arr)
				bot->BlackListAdd(id);
			logging::INFO("添加成功 <BlackList>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("黑名单更新好了捏"));
			return true;
		}

		if (command == "delete" || command == "del")
		{
			for (const auto& id : arr)
				bot->BlackListDelete(id);
			logging::INFO("删除成功 <BlackList>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("黑名单更新好了捏"));
			return true;
		}
	}

	logging::INFO("未知命令 <BlackList>: " + command + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Plain(command + "是什么指令捏，不知道捏"));
	return false;
}