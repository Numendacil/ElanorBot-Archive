#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <random>
#include <mirai/defs/MessageChain.hpp>
#include <mirai/events/events.hpp>

namespace httplib_ssl_zlib
{
	class Result;
}

namespace Common
{
	extern std::mt19937 rng_engine;
	extern const std::string WhiteSpace;
	extern const std::string MediaFilePath;

	void Init(void);

	std::string exec(const std::vector<std::string>& cmd);

	void ReplaceMark(std::string &);
	void ToLower(std::string &);
	int ToBool(const std::string&);
	int Tokenize(std::vector<std::string>&, std::string, int = -1);

	bool CheckHttpResponse(const httplib_ssl_zlib::Result& result, const std::string& Caller = "");

	std::string GetDescription(const Cyan::GroupMessage &, bool = true);
	std::string GetDescription(const Cyan::FriendMessage &, bool = true);


	void SendGroupMessage(const Cyan::GroupMessage &gm, const Cyan::MessageChain &msg);
	void QuoteGroupMessage(const Cyan::GroupMessage &gm, const Cyan::MessageChain &msg);
}

#endif
