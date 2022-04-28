#ifndef _GROUP_COMMAND_BASE_HPP_
#define _GROUP_COMMAND_BASE_HPP_

#include <string>
#include <mirai/MiraiBot.hpp>
#include <mirai/events/Message.hpp>
#include <mirai/defs/MessageChain.hpp>
#include "../ElanorBot.hpp"

class GroupCommandBase
{
public:
	virtual int AuthRequirement(void) { return 0; }
	virtual int Priority(void) { return 0; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>&) = 0;
	virtual bool Execute(const Cyan::GroupMessage&, Cyan::MiraiBot&, ElanorBot& bot, const std::vector<std::string>&) = 0;
};

#endif