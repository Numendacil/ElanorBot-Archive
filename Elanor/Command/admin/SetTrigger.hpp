#ifndef _SET_TRIGGER_HPP_
#define _SET_TRIGGER_HPP_

#include <Command/GroupCommandBase.hpp>

class SetTrigger : public GroupCommandBase
{
public:
	virtual int AuthRequirement(void) { return 50; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens);
};

#endif