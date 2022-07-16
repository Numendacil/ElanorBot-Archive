#ifndef _PJSK_UPDATE_HPP_
#define _PJSK_UPDATE_HPP_

#include <Command/GroupCommandBase.hpp>

class pjskUpdate : public GroupCommandBase
{
public:
	virtual int Permission(void) { return 50; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

#endif