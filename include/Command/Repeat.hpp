#ifndef _REPEAT_HPP_
#define _REPEAT_HPP_

#include "GroupCommandBase.hpp"

class Repeat : public GroupCommandBase
{
public:
	virtual int Priority(void) { return 0; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens) override;
};

#endif