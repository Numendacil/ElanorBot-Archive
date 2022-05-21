#ifndef _PJSK_SONG_GUESS_HPP_
#define _PJSK_SONG_GUESS_HPP_

#include "../GroupCommandBase.hpp"

class pjskSongGuess : public GroupCommandBase
{
public:
	virtual bool Parse(const Cyan::MessageChain& msg, std::vector<std::string>& tokens);
	virtual bool Execute(const Cyan::GroupMessage& gm, std::shared_ptr<ElanorBot> bot, const std::vector<std::string>& tokens);
};

#endif