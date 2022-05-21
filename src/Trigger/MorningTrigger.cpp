#include "Trigger/MorningTrigger.hpp"
#include "ElanorBot.hpp"
#include "MessageQueue.hpp"
#include "third-party/log.h"

using namespace std;
using namespace Cyan;

void MorningTrigger::Action(shared_ptr<MiraiBot> client, ElanorBot* bot)
{
	assert(bot);
	auto GroupInfo = client->GetGroupConfig(bot->gid);
	logging::INFO("Send morning <MorningTrigger>\t-> " + GroupInfo.Name + "(" + to_string(bot->gid.ToInt64()) + ")");
	MessageQueue::GetInstance().Push(bot->gid, MessageChain().Plain("起床啦！"));
}