#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <mirai/third-party/nlohmann/json.hpp>
#include <third-party/json.hpp>
#include <random>
#include <optional>
#include <mirai/defs/MessageChain.hpp>
#include <mirai/events/events.hpp>

namespace httplib_ssl_zlib
{
	class Result;
}

namespace Utils
{
	class BotConfig
	{
	private:
		nlohmann::json config;

	public:
		BotConfig() = default;
		BotConfig(const nlohmann::json& config) { this->config = config;}
		bool FromFile(const std::string& filepath);

		template<typename T>
		T Get(const nlohmann::json::json_pointer& key, const T& value) const
		{
			if (this->config.contains(key))
				return this->config.at(key).get<T>();
			else
				return value;
		}
		template<typename T>
		std::optional<T> Get(const nlohmann::json::json_pointer& key) const
		{
			if (this->config.contains(key))
				return this->config.at(key).get<T>();
			else
				return nullopt;
		}
	};
}

namespace Utils
{
	extern std::mt19937 rng_engine;
	extern BotConfig Configs;

	void Init(const std::string& config_path);

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
