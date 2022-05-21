#ifndef _GROUP_COMMAND_BASE_HPP_
#define _GROUP_COMMAND_BASE_HPP_

#include <string>
#include <mirai/MiraiBot.hpp>
#include <mirai/events/Message.hpp>
#include <mirai/defs/MessageChain.hpp>

class ElanorBot;

class GroupCommandBase
{
public:
	virtual int AuthRequirement(void) { return 0; }
	virtual int Priority(void) { return 10; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) = 0;
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens) = 0;

	virtual ~GroupCommandBase() = default;
};

#endif