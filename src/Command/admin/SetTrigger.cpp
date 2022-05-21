#include "Command/admin/SetTrigger.hpp"
#include "Utils.hpp"
#include "third-party/log.h"
#include "ElanorBot.hpp"
#include "Factory.hpp"

using namespace std;
using namespace Cyan;


bool SetTrigger::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#trig"))
	{
		if (Utils::Tokenize(token, str) < 2)
			return false;
		Utils::ToLower(token[0]);
		if (token[0] == "#trig" || token[0] == "#trigger" || token[0] == "#触发器")
			return true;
	}
	return false;
}

bool SetTrigger::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling SetTrigger <SetTrigger>" + Utils::GetDescription(gm));
	string command = token[1];
	Utils::ToLower(command);

	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档 <SetTrigger>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#trigger set [trigger] {on/off}\n#trigger status [trigger]\n#trigger list"));
		return true;
	}

	if (command == "set" || command == "status" || command == "list")
	{
		if (command == "list")
		{
			auto list = Factory<TriggerBase>::GetKeyList();
			string message = "触发器列表";
			for (const auto& s : list)
			{
				message += "\n" + s + ": " + ((bot->GetTriggerStatus(s))? "✅" : "❌");
			}
			logging::INFO("输出触发器列表 <SetTrigger>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(message));
			return true;
		}

		if (token.size() < 3)
		{
			logging::INFO("缺少参数[trigger] <SetTrigger>: " + token[1] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("缺少参数[trigger]，是被你吃了嘛"));
			return false;
		}

		string target = token[2];
		if (!bot->ExistTrigger(target))
		{
			logging::INFO("无效参数[trigger] <SetTrigger>: " + target + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(target + "是哪个触发器捏，不知道捏"));
			return false;
		}

		if (command == "status")
		{
			logging::INFO("输出触发器状态 <SetTrigger>: " + target + string((bot->GetTriggerStatus(target))? "✅" : "❌") + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(target + " 当前状态: " + string((bot->GetTriggerStatus(target))? "✅" : "❌")));
			return true;
		}

		if (token.size() < 4)
		{
			logging::INFO("缺少参数{on/off} <SetAuth>: " + token[1] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("on还是off, 到底要选哪个呢🔉到底要选哪个呢🔉到底要选哪个呢🔉"));
			return false;
		}

		switch(Utils::ToBool(token[3]))
		{
		case 1:
			bot->TriggerOn(target);
			logging::INFO("Trigger on ✅ <SetTrigger>: " + target + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("已启动 " + target + " ✅"));
			return true;

		case 0:
			bot->TriggerOff(target);
			logging::INFO("Trigger off ❌ <SetTrigger>: " + target + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("已关闭 " + target + " ❌"));
			return true;

		default:
			logging::INFO("未知选项 <SetTrigger>: " + token[3] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(token[3] + "是什么意思捏，看不懂捏"));
			return false;
		}
	}


	logging::INFO("未知指令 <SetTrigger>: " + token[1] + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Plain(token[1] + "是什么东西捏，不知道捏"));
	return false;
}