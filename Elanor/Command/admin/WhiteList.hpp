#ifndef _WHITE_LIST_HPP_
#define _WHITE_LIST_HPP_

#include <Command/GroupCommandBase.hpp>

class WhiteList : public GroupCommandBase
{
public:
	virtual int AuthRequirement(void) override{ return 100; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens) override;
};

#endif