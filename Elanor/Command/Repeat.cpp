#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <State/LastMessage.hpp>

#include "Repeat.hpp"

using namespace std;

namespace GroupCommand
{

bool Repeat::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.ToString();
	if (str.empty() || str == "[]")
		return false;
	tokens.clear();
	tokens.push_back(str);
	return true;
}

bool Repeat::Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const vector<string>& tokens) 
{
	Cyan::MessageChain LastMsg;
	bool Repeated;
	auto state = group.GetState<State::LastMessage>("LastMessage");
	state->Get(LastMsg, Repeated);

	if (tokens[0] == LastMsg.ToString())
	{
	//	logging::INFO("有人复读 <Repeat>: " + tokens[0] + Utils::GetDescription(gm));
		if (!Repeated)
		{
			uniform_int_distribution rng_repeat(1, 10);
			if (rng_repeat(Utils::rng_engine) == 1)
			{
				state->Set(gm.MessageChain, true);
				Bot::Client::GetClient().Send(gm.Sender.Group.GID, gm.MessageChain);
				logging::INFO("bot复读成功 <Repeat>" + Utils::GetDescription(gm, false));
				return true;
			}
		}
	}
	else
	{
		state->Set(gm.MessageChain, false);
	}
	return false;
}

}