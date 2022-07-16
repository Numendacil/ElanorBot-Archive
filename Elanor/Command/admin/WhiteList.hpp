#ifndef _WHITE_LIST_HPP_
#define _WHITE_LIST_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class WhiteList : public GroupCommandBase
{
public:
	virtual int Permission(void) override { return 100; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif