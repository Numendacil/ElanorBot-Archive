#include "GroupList.hpp"
#include "mirai/defs/QQType.hpp"
#include <cassert>
#include <filesystem>
#include <mutex>
#include <vector>

namespace Bot
{

GroupList::GroupList(Cyan::QQ_t owner_id, 
		const std::vector<std::pair<std::string, int>>& command_list,
		const std::vector<std::pair<std::string, bool>>& trigger_list) : 
		command_list(command_list), trigger_list(trigger_list), owner(owner_id)
{
	string path = "./bot";
	for (const auto & entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_regular_file())
		{
			try
			{
				Cyan::GID_t gid = (Cyan::GID_t)std::stol(entry.path().stem());
				this->group_list.try_emplace(gid, gid, owner_id, this->command_list, this->trigger_list);
			}
			catch(const std::logic_error& e)
			{
				logging::WARN("Unexpected file found in ./bot directory: " + string(entry.path().filename()));
			}
		}
	}
}

Group& GroupList::GetGroup(Cyan::GID_t gid)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	auto it = this->group_list.find(gid);
	if (it != this->group_list.end())
		return it->second;
	else
	{
		auto result = this->group_list.try_emplace(gid, gid, this->owner, this->command_list, this->trigger_list);
		assert(result.second);
		return result.first->second;
	}

}

std::vector<Group*> GroupList::GetAllGroups()
{
	std::lock_guard<std::mutex> lk(this->mtx);
	std::vector<Group*> v;
	v.reserve(this->group_list.size());
	for (auto& p : this->group_list)
		v.push_back(&p.second);
	return v;
}

}