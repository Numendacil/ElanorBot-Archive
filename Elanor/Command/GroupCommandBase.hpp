#ifndef _GROUP_COMMAND_BASE_HPP_
#define _GROUP_COMMAND_BASE_HPP_

#include <string>
#include <mirai/events/Message.hpp>

class Group;

namespace GroupCommand
{

class GroupCommandBase
{
public:
	virtual int Permission(void) { return 0; }
	virtual int Priority(void) { return 10; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) = 0;
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) = 0;

	virtual ~GroupCommandBase() = default;
};

}

#endif