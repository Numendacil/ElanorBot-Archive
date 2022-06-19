#include "third-party/log.h"
#include "third-party/json.hpp"
#include "third-party/httplib.hpp"
#include "Command/image/Petpet.hpp"
#include "Utils.hpp"
#include "ElanorBot.hpp"

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

bool Petpet::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pet"))
	{
		Utils::Tokenize(token, str);
		Utils::ToLower(token[0]);
		if (token[0] == "#pet" || token[0] == "#摸摸")
			return true;
	}
	return false;
}

bool Petpet::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	logging::INFO("Calling Petpet <Petpet>" + Utils::GetDescription(gm));
	int64_t target = -1;
	if (token.size() > 1)
	{
		if (token[1].empty())
		{
			logging::INFO("参数为空 <Petpet>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("QQ号看不见捏，怎么会事捏"));
			return false;
		}
		if (token[1] == "help" || token[1] == "h" || token[1] == "帮助")
		{
			logging::INFO("帮助文档 <Petpet>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#pet [QQ]"));
			return true;
		}
		else
		{
			try
			{
				target = stol(token[1]);
			}
			catch (const logic_error& e)
			{
				logging::INFO("无效参数 <Petpet>: " + token[1] + Utils::GetDescription(gm, false));
				Utils::SendGroupMessage(gm, MessageChain().Plain(token[1] + "是个锤子QQ号"));
				return false;
			}
		}
	}
	try
	{
		auto AtMsg = gm.MessageChain.GetFirst<AtMessage>();
		target = AtMsg.Target().ToInt64();
	}
	catch(const runtime_error& e) {}

	if (target != -1)
	{
		if (target == bot->client->GetBotQQ().ToInt64())
		{
			uniform_int_distribution<int> rngroll(0, 4);
			if (!rngroll(Utils::rng_engine))
			{
				logging::INFO("你吗 <Petpet>" + Utils::GetDescription(gm, false));
				Utils::SendGroupMessage(gm, MessageChain().Plain("摸你吗个头，爬"));
				return true;
			}
		}

		httplib_ssl_zlib::Client  cli(Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000"));
		auto result = cli.Get("/gen/pet/", {{"qq", to_string(target)}}, {{"Accept-Encoding", "gzip"}});
		if (!Utils::CheckHttpResponse(result, "Petpet"))
		{
			Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}
		json msg = json::parse(result->body);
		
		assert(msg.contains("result"));
		logging::INFO("上传gif <Petpet>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Image({"", "", "", msg["result"]}));
		return true;
	}
	else
	{
		logging::INFO("缺少target <Petpet>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("你搁这摸空气呢"));
		return false;
	}
}