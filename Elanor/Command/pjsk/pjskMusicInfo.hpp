#ifndef _PJSK_MUSIC_INFO_HPP_
#define _PJSK_MUSIC_INFO_HPP_

#include <Command/GroupCommandBase.hpp>

class pjskMusicInfo : public GroupCommandBase
{
public:
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens);
};

#endif