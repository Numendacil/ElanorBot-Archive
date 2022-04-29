#ifndef _ROLL_DICE_HPP_
#define _ROLL_DICE_HPP_

#include "GroupCommandBase.hpp"

class RollDice : public GroupCommandBase
{
public:
	virtual int Priority(void) override { return 10; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<Cyan::MiraiBot> client, shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens) override;
};

#endif