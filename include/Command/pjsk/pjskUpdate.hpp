#ifndef _PJSK_UPDATE_HPP_
#define _PJSK_UPDATE_HPP_

#include "../GroupCommandBase.hpp"

class pjskUpdate : public GroupCommandBase
{
public:
	virtual int AuthRequirement(void) { return 50; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens);
};

#endif