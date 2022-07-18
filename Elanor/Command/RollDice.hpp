#ifndef _ROLL_DICE_HPP_
#define _ROLL_DICE_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class RollDice : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "RollDice";

	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif