#ifndef _ANSWER_HPP_
#define _ANSWER_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class Answer : public GroupCommandBase
{
public:
	virtual int Permission(void) override { return 0; }
	virtual int Priority(void) override { return 5; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif