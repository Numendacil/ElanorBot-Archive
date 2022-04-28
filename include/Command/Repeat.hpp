#ifndef _REPEAT_HPP_
#define _REPEAT_HPP_

#include "GroupCommandBase.hpp"

class Repeat : public GroupCommandBase
{
public:
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>&) override;
	virtual bool Execute(const Cyan::GroupMessage&, Cyan::MiraiBot&, ElanorBot& bot, const std::vector<std::string>&) override;
};

#endif