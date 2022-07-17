#include <ThirdParty/log.h>
#include <ThirdParty/json.hpp>
#include <ThirdParty/httplib.hpp>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include <mirai.h>

#include "Petpet.hpp"

using namespace std;
using json = nlohmann::json;

namespace GroupCommand
{

bool Petpet::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pet"))
	{
		Utils::Tokenize(tokens, str);
		Utils::ToLower(tokens[0]);
		if (tokens[0] == "#pet" || tokens[0] == "#摸摸")
			return true;
	}
	return false;
}

bool Petpet::Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const vector<string>& tokens) 
{
	logging::INFO("Calling Petpet <Petpet>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	int64_t target = -1;
	if (tokens.size() > 1)
	{
		if (tokens[1].empty())
		{
			logging::INFO("参数为空 <Petpet>" + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("QQ号看不见捏，怎么会事捏"));
			return false;
		}
		if (tokens[1] == "help" || tokens[1] == "h" || tokens[1] == "帮助")
		{
			logging::INFO("帮助文档 <Petpet>" + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("usage:\n#pet [QQ]"));
			return true;
		}
		else
		{
			try
			{
				target = stol(tokens[1]);
			}
			catch (const logic_error& e)
			{
				logging::INFO("无效参数 <Petpet>: " + tokens[1] + Utils::GetDescription(gm, false));
				client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain(tokens[1] + "是个锤子QQ号"));
				return false;
			}
		}
	}
	try
	{
		auto AtMsg = gm.MessageChain.GetFirst<Cyan::AtMessage>();
		target = AtMsg.Target().ToInt64();
	}
	catch(const runtime_error& e) {}

	if (target != -1)
	{
		if (target == client.Call(&Cyan::MiraiBot::GetBotQQ).ToInt64());
		{
			uniform_int_distribution<int> rngroll(0, 4);
			if (!rngroll(Utils::rng_engine))
			{
				logging::INFO("你吗 <Petpet>" + Utils::GetDescription(gm, false));
				client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("摸你吗个头，爬"));
				return true;
			}
		}

		httplib_ssl_zlib::Client cli(Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000"));
		auto result = cli.Get("/gen/pet/", {{"qq", to_string(target)}}, {{"Accept-Encoding", "gzip"}});
		if (!Utils::CheckHttpResponse(result, "Petpet"))
		{
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}
		json msg = json::parse(result->body);
		
		assert(msg.contains("result"));
		logging::INFO("上传gif <Petpet>" + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Image({.Base64 = msg["result"]}));
		return true;
	}
	else
	{
		logging::INFO("缺少target <Petpet>" + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("你搁这摸空气呢"));
		return false;
	}
}

}