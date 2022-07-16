#ifndef _PJSK_SONG_GUESS_HPP_
#define _PJSK_SONG_GUESS_HPP_

#include <Command/GroupCommandBase.hpp>

namespace GroupCommand
{

class pjskSongGuess : public GroupCommandBase
{
public:
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens) override;
	virtual bool Execute(const Cyan::GroupMessage& gm, Group& group, const std::vector<std::string>& tokens) override;
};

}

#endif