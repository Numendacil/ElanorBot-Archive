#include "utils/log.h"
#include "Command/Repeat.hpp"
#include "Common.hpp"
#include "State/LastMessage.hpp"

using namespace std;
using namespace Cyan;

bool Repeat::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.ToString();
	if (str.empty() || str == "[]")
		return false;
	token.clear();
	token.push_back(str);
	return true;
}

bool Repeat::Execute(const GroupMessage& gm, MiraiBot& client, ElanorBot& bot, const vector<string>& token)
{
	auto state = bot.GetState<LastMessage>("Repeat");
	auto lock = state->GetLock();

	if (token[0] == state->LastMsg.ToString())
	{
		logging::INFO("有人复读<Repeat>: " + token[0] + Common::GetDescription(gm));
		if (!state->Repeated)
		{
			uniform_int_distribution rng_repeat(1, 3);
			if (rng_repeat(Common::rng_engine) == 1)
			{
				state->Repeated = true;
				Common::SendGroupMessage(gm, gm.MessageChain);
				logging::INFO("bot复读成功<Repeat>" + Common::GetDescription(gm, false));
				return true;
			}
		}
	}
	else
	{
		state->LastMsg = gm.MessageChain;
		state->Repeated = false;
	}
	return false;
}