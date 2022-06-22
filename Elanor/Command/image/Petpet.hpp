#ifndef _PETPET_HPP_
#define _PETPET_HPP_

#include <Command/GroupCommandBase.hpp>

class Petpet: public GroupCommandBase
{
public:
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens) override;
};

#endif