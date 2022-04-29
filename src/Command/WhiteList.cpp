#include <stdexcept>
#include <mirai/exceptions/exceptions.hpp>
#include "utils/log.h"
#include "Command/WhiteList.hpp"
#include "Common.hpp"

using namespace std;
using namespace Cyan;

bool WhiteList::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Common::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#whitelist") + 1)
	{
		Common::ToLower(str);
		if (Common::Tokenize(token, str) < 2)
			return false;
		if (token[0] == "#whitelist" || token[0] == "#白名单")
			return true;
	}
	return false;
}

bool WhiteList::Execute(const GroupMessage& gm, shared_ptr<MiraiBot> client, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling whitelist<WhiteList>" + Common::GetDescription(gm));
	string command = token[1];
	if (command == "help" || command == "h")
	{
		logging::INFO("帮助文档<WhiteList>" + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("usage:\n#whitelist [add/delete/exist] <QQ>\n#whitelist [clear/clean/list]"));
		return true;
	}

	if (command == "clear")
	{
		bot->WhiteListClear();
		logging::INFO("清除成功<WhiteList>" + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("白名单归零了捏"));
		return true;
	}

	if (command == "clean")
	{
		auto list = bot->GetWhiteList();
		for (const auto& id : list)
		{
			try
			{
				client->GetGroupMemberInfo(gm.Sender.Group.GID, id);
			}
			catch (const MiraiApiHttpException& e)	// No such member
			{
				bot->WhiteListDelete(id);
			}
		}
		logging::INFO("整理成功<WhiteList>" + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("白名单打扫好了捏"));
		return true;
	}

	if (command == "list")
	{
		auto list = bot->GetWhiteList();
		string msg = "本群白名单:\n";
		for (const auto& id : list)
		{
			try
			{
				auto profile = client->GetGroupMemberInfo(gm.Sender.Group.GID, id);
				msg += to_string(id.ToInt64()) + " (" + profile.MemberName + ")\n";
			}
			catch (const MiraiApiHttpException& e)	// No such member
			{
				msg += to_string(id.ToInt64()) + " (目前不在群内)\n";
			}
		}
		logging::INFO("输出名单<WhiteList>" + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain(msg));
		return true;
	}

	if (command == "exist" || command == "add" || command == "del" || command == "delete")
	{
		auto AtMsg = gm.MessageChain.GetAll<AtMessage>();
		if (token.size() + AtMsg.size() < 3)
		{
			logging::INFO("参数数目错误<WhiteList>: " + command + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("缺少参数 <QQ>，是被你吃了嘛"));
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
			logging::INFO("无效参数<WhiteList>: " + token[i] + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain(token[i] + "是个锤子QQ号"));
			return false;
		}
		for (const auto& p : AtMsg)
			arr.push_back(p.Target());

		if (command == "exist")
		{
			string msg = "查询结果:\n";
			for (const auto& id : arr)
			{
				msg += to_string(id.ToInt64()) + ((bot->IsWhiteList(id))? " 在白名单中\n" : " 不在白名单中\n");
			}
			logging::INFO("查询成功<WhiteList>" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain(msg));
			return true;
		}

		if (command == "add")
		{
			for (const auto& id : arr)
				bot->WhiteListAdd(id);
			logging::INFO("添加成功<WhiteList>" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("白名单更新好了捏"));
			return true;
		}

		if (command == "delete" || command == "del")
		{
			for (const auto& id : arr)
				bot->WhiteListDelete(id);
			logging::INFO("删除成功<WhiteList>" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("白名单更新好了捏"));
			return true;
		}
	}

	logging::INFO("未知命令<WhiteList>: " + command + Common::GetDescription(gm, false));
	Common::SendGroupMessage(gm, MessageChain().Plain(command + "是什么指令捏，不知道捏"));
	return false;
}