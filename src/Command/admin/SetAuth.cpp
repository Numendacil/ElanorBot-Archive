#include "Command/admin/SetAuth.hpp"
#include "Utils.hpp"
#include "third-party/log.h"
#include "ElanorBot.hpp"
#include "Factory.hpp"

using namespace std;
using namespace Cyan;


bool SetAuth::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#auth"))
	{
		if (Utils::Tokenize(token, str) < 2)
			return false;
		Utils::ToLower(token[0]);
		if (token[0] == "#auth" || token[0] == "#权限")
			return true;
	}
	return false;
}

bool SetAuth::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling Auth <SetAuth>" + Utils::GetDescription(gm));
	string command = token[1];
	Utils::ToLower(command);

	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档 <SetAuth>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#auth set [command] [level]\n#auth {reset/show} [command]\n#auth list"));
		return true;
	}

	if (command == "set" || command == "reset" || command == "show" || command == "list")
	{
		if (command == "list")
		{
			auto list = Factory<GroupCommandBase>::GetKeyList();
			string message = "指令权限列表";
			for (const auto& s : list)
			{
				message += "\n" + s + ": " + to_string(bot->GetCommandAuth(s));
			}
			logging::INFO("输出权限列表 <SetAuth>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(message));
			return true;
		}

		if (token.size() < 3)
		{
			logging::INFO("缺少参数[command] <SetAuth>: " + token[1] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("缺少参数[command]，是被你吃了嘛"));
			return false;
		}
		
		string target = token[2];
		if (!bot->ExistCommand(target))
		{
			logging::INFO("无效参数[command] <SetAuth>: " + target + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(target + "是哪个指令捏，不知道捏"));
			return false;
		}

		if (command == "reset")
		{
			auto c = Factory<GroupCommandBase>::Make(target);
			bot->UpdateAuth(target, c->AuthRequirement());
			logging::INFO("重置权限 <SetAuth>: " + target + "(" + to_string(c->AuthRequirement()) + ")" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("重置 " + target + " 的权限等级为 " + to_string(c->AuthRequirement())));
			return true;
		}

		if (command == "show")
		{
			logging::INFO("输出指令权限 <SetAuth>: " + target + "(" + to_string(bot->GetCommandAuth(target)) + ")" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(target + " 的权限等级为 " + to_string(bot->GetCommandAuth(target))));
			return true;
		}

		if (token.size() < 4)
		{
			logging::INFO("缺少参数[level] <SetAuth>: " + token[1] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("缺少参数[level]，是被你吃了嘛"));
			return false;
		}

		int auth;
		try
		{
			auth = stoi(token[3]);
			if (auth > 100 || auth < 0)
			{
				logging::INFO("无效参数[level] <SetAuth>: " + token[3] + Utils::GetDescription(gm, false));
				Utils::SendGroupMessage(gm, MessageChain().Plain("权限等级必须在0到100之间捏"));
				return false;
			}
		}
		catch(const logic_error& e)
		{
			logging::INFO("无效参数[level] <SetAuth>: " + token[3] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("你觉得" + token[3] + "很像个整数么"));
			return false;
		}

		if ((bot->GetCommandAuth(target) >= 50 || auth >= 50) && gm.Sender.QQ != bot->suid)
		{
			logging::INFO("权限不足 <SetAuth>: 无法修改该指令的权限(" + target + ")" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("你没资格啊，你没资格\n正因如此你没资格啊，你没资格"));
			return false;
		}

		bot->UpdateAuth(target, auth);
		logging::INFO("更新权限 <SetAuth>: " + target + "(" + token[3] + ")" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("指令权限更新好了捏"));
		return true;
	}


	logging::INFO("未知命令 <SetAuth>: " + token[1] + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Plain(token[1] + "是什么指令捏，不知道捏"));
	return false;
}