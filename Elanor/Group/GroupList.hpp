#ifndef _GROUP_LIST_HPP_
#define _GROUP_LIST_HPP_

#include <mirai/defs/QQType.hpp>
#include <Group/Group.hpp>
#include <map>
#include <mutex>
#include <vector>

namespace Bot
{

class GroupList
{
protected:
	std::map<Cyan::GID_t, Group> group_list;
	const std::vector<std::pair<std::string, int>>& command_list;
	const std::vector<std::pair<std::string, bool>>& trigger_list;
	Cyan::QQ_t owner;

	mutable std::mutex mtx;
	
public:
	GroupList(Cyan::QQ_t owner_id, 
		const std::vector<std::pair<std::string, int>>& command_list,
		const std::vector<std::pair<std::string, bool>>& trigger_list);

	Group& GetGroup(Cyan::GID_t gid);
	std::vector<Group*> GetAllGroups();

};

}

#endif