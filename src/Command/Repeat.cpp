#include "third-party/log.h"
#include "Command/Repeat.hpp"
#include "Utils.hpp"
#include "ElanorBot.hpp"
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

bool Repeat::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	MessageChain LastMsg;
	bool Repeated;
	auto state = bot->GetState<LastMessage>("Repeat");
	state->Get(LastMsg, Repeated);

	if (token[0] == LastMsg.ToString())
	{
	//	logging::INFO("有人复读 <Repeat>: " + token[0] + Utils::GetDescription(gm));
		if (!Repeated)
		{
			uniform_int_distribution rng_repeat(1, 10);
			if (rng_repeat(Utils::rng_engine) == 1)
			{
				state->Set(gm.MessageChain, true);
				Utils::SendGroupMessage(gm, gm.MessageChain);
				logging::INFO("bot复读成功 <Repeat>" + Utils::GetDescription(gm, false));
				return true;
			}
		}
	}
	else
	{
		if (gm.Sender.QQ == bot->client->GetBotQQ())
			state->Set(gm.MessageChain, true);
		else
			state->Set(gm.MessageChain, false);
	}
	return false;
}