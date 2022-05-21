#ifndef _RECALL_HPP_
#define _RECALL_HPP_

#include "GroupCommandBase.hpp"

class Recall : public GroupCommandBase
{
public:
	virtual int AuthRequirement(void) { return 50; }
	virtual int Priority(void) override { return 20; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens) override;
};

#endif