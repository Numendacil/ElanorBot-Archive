#ifndef _ROLL_DICE_HPP_
#define _ROLL_DICE_HPP_

#include "GroupCommandBase.hpp"

class RollDice : public GroupCommandBase
{
public:
	virtual int Priority(void) override { return 10; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>&) override;
	virtual bool Execute(const Cyan::GroupMessage&, Cyan::MiraiBot&, ElanorBot& bot, const std::vector<std::string>&) override;
};

#endif