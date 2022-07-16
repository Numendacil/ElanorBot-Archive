#ifndef _PETPET_HPP_
#define _PETPET_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class Petpet: public GroupCommandBase
{
public:
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif