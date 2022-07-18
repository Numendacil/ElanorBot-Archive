#include <ThirdParty/log.h>
#include <State/Activity.hpp>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include "Answer.hpp"

using namespace std;

namespace GroupCommand
{

bool Answer::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str[0] != '.')
		return false;
	tokens.clear();
	tokens.push_back(str.substr(1));
	return true;
}

bool Answer::Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const vector<string>& tokens) 
{
	auto state = group.GetState<State::Activity>();
	if (!state->HasActivity())
		return false;
	if (state->GetActivityName() == "pjsk")
	{
		assert(!tokens.empty());
		logging::INFO("<Answer: pjsk>: " + tokens[0] + Utils::GetDescription(gm));
		state->AddAnswer({tokens[0], gm.MessageId()});
		return true;
	}
	return false;
}

}