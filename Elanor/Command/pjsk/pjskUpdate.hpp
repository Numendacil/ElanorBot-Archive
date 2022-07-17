#ifndef _PJSK_UPDATE_HPP_
#define _PJSK_UPDATE_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class pjskUpdate : public GroupCommandBase
{
public:
	virtual int Permission(void) override { return 50; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif