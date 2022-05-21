#ifndef _COMMAND_AUTH_HPP_
#define _COMMAND_AUTH_HPP_

#include "../GroupCommandBase.hpp"

class SetAuth : public GroupCommandBase
{
public:
	virtual int AuthRequirement(void) { return 50; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens);
};

#endif