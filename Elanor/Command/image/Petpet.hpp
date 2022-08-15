#ifndef _PETPET_HPP_
#define _PETPET_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class Petpet: public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "Petpet";

	virtual bool Parse(const Mirai::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Mirai::GroupMessageEvent& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif