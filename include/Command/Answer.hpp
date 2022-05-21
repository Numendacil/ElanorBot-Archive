#ifndef _ANSWER_HPP_
#define _ANSWER_HPP_

#include "GroupCommandBase.hpp"

class Answer : public GroupCommandBase
{
public:
	virtual int AuthRequirement(void) { return 0; }
	virtual int Priority(void) { return 5; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens);
};

#endif