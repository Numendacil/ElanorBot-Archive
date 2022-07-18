#ifndef _PIXIV_HPP_
#define _PIXIV_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class Pixiv : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "Pixiv";

	virtual int Priority(void) override { return 10; }
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif