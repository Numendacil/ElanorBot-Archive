#include "utils/log.h"
#include "Command/RollDice.hpp"
#include "Common.hpp"

using namespace std;
using namespace Cyan;

bool RollDice::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Common::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#roll") + 1)
	{
		Common::ToLower(str);
		if (Common::Tokenize(token, str, 2) < 2)
			return false;
		if (token[0] == "#roll")
			return true;
	}
	return false;
}

bool RollDice::Execute(const GroupMessage& gm, MiraiBot& client, ElanorBot& bot, const vector<string>& token)
{
	int i = 0;
	int j = 0;
	int result[10];
	assert(token.size() > 1);
	string command = token[1];
	if (command == "help" || command == "h")
	{
		logging::INFO("帮助文档<RollDice>" + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("usage:\n#roll <x>D<y>"));
		return true;
	}
	while (command[i + j] >= '0' && command[i + j] <= '9')
		j++;
	if (command[i + j] == 'd')
	{
		try
		{
			int round;
			if (j == 0)
				round = 1;
			else
				round = stoi(command.substr(i, j));
			if (round > 10)
			{
				logging::INFO("投掷次数错误<RollDice>: round = " + to_string(round) + Common::GetDescription(gm, false));
				Common::SendGroupMessage(gm, MessageChain().Plain("骰子太多啦！"));
				return false;
			}
			if (round < 1)
			{
				logging::INFO("投掷次数错误<RollDice>: round = " + to_string(round) + Common::GetDescription(gm, false));
				Common::SendGroupMessage(gm, MessageChain().Plain("骰子不见了捏，怎么会事捏"));
				return false;
			}


			i = i + j + 1;
			j = 0;
			while (i + j < command.length() && command[i + j] >= '0' && command[i + j] <= '9')
				j++;
			if (j)
			{
				int max = stoi(command.substr(i, j));
				uniform_int_distribution<int> rngroll(0, max);
				int ans = 0;
				string msg = "";
				for (int l = 0; l < round; ++l)
				{
					result[i] = rngroll(Common::rng_engine);
					ans += result[i];
					msg += (l)? " + " + to_string(result[i]) : to_string(result[i]);
				}
				msg += " = ";
				logging::INFO("随机数生成<RollDice>: " + msg + to_string(ans) + Common::GetDescription(gm, false));
				if (round == 1)
					Common::SendGroupMessage(gm, MessageChain().Plain(gm.Sender.MemberName + " 掷出了: " + to_string(ans)));
				else
					Common::SendGroupMessage(gm, MessageChain().Plain(gm.Sender.MemberName + " 掷出了: " + msg + to_string(ans)));
				return true;
			}
		}
		catch (out_of_range &)
		{
			logging::INFO("数字溢出<RollDice>" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("数字太、太大了"));
			return false;
		}
	}

	logging::INFO("格式错误<RollDice>" + Common::GetDescription(gm, false));
	Common::SendGroupMessage(gm, MessageChain().Plain("格式错误，使用示例 #roll 1D100"));
	return false;
}