#ifndef _AT_HPP_
#define _AT_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class AtBot : public GroupCommandBase
{
public:
	virtual int Priority(void) override { return 1; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

}
#endif