#include <third-party/log.h>
#include <Command/Answer.hpp>
#include <State/Activity.hpp>
#include <Utils/Utils.hpp>
#include <app/ElanorBot.hpp>

using namespace std;
using namespace Cyan;

bool Answer::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str[0] != '.')
		return false;
	token.clear();
	token.push_back(str.substr(1));
	return true;
}

bool Answer::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	auto state = bot->GetState<Activity>("Activity");
	if (!state->HasActivity())
		return false;
	if (state->GetActivityName() == "pjsk")
	{
		assert(!token.empty());
		logging::INFO("<Answer: pjsk>: " + token[0] + Utils::GetDescription(gm));
		state->AddAnswer({token[0], gm.MessageId()});
		return true;
	}
	return false;
}