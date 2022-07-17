#ifndef _REPEAT_HPP_
#define _REPEAT_HPP_

#include <Command/GroupCommandBase.hpp>


namespace GroupCommand
{

class Repeat : public GroupCommandBase
{
public:
	virtual int Priority(void) override { return 0; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif