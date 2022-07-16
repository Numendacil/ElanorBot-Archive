#ifndef _PJSK_CHART_HPP_
#define _PJSK_CHART_HPP_

#include <Command/GroupCommandBase.hpp>

class pjskChart : public GroupCommandBase
{
public:
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

#endif