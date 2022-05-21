#ifndef _BILILIVE__HPP_
#define _BILILIVE_HPP_

#include "GroupCommandBase.hpp"

class Bililive: public GroupCommandBase
{
	virtual int AuthRequirement(void) override{ return 10; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens) override;
};

#endif