#ifndef _IMAGE_SEARCH_HPP_
#define _IMAGE_SEARCH_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class ImageSearch : public GroupCommandBase
{
public:

	static constexpr std::string_view _NAME_ = "ImageSearch";

	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif