#ifndef _BILILIVE__HPP_
#define _BILILIVE_HPP_

#include <Command/GroupCommandBase.hpp>


namespace GroupCommand
{

class Bililive: public GroupCommandBase
{
	virtual int Permission(void) override{ return 10; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif