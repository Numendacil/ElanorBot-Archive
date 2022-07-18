#include <ThirdParty/log.h>
#include <Group/GroupList.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <State/TriggerStatus.hpp>
#include <mirai.h>

#include "MorningTrigger.hpp"

using namespace std;

namespace Trigger
{

void MorningTrigger::Action()
{
	auto groups_ptr = this->groups.lock();
	if (!groups_ptr)
		return;
	auto group_list = groups_ptr->GetAllGroups();
	Bot::Client& client = Bot::Client::GetClient();
	for (const auto& p : group_list)
	{
		auto enabled = p->GetState<State::TriggerStatus>();
		if (enabled->GetTriggerStatus("Morning"))
		{
			auto GroupInfo = client.Call(&Cyan::MiraiBot::GetGroupConfig, p->gid);
			logging::INFO("Send morning <MorningTrigger>\t-> " + GroupInfo.Name + "(" + to_string(p->gid.ToInt64()) + ")");
			client.Send(p->gid, Cyan::MessageChain().Plain("起床啦！"));
		}
	}
}

}