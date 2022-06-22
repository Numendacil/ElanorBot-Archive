#include <third-party/log.h>
#include <third-party/json.hpp>
#include <third-party/httplib.hpp>
#include <Command/image/Choyen.hpp>
#include <Utils/Utils.hpp>
#include <app/ElanorBot.hpp>

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

bool Choyen::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#choyen"))
	{
		if (Utils::Tokenize(token, str, 3) < 2)
			return false;
		Utils::ToLower(token[0]);
		if (token[0] == "#choyen" || token[0] == "#红字白字")
			return true;
	}
	return false;
}

bool Choyen::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	logging::INFO("Calling Choyen <Choyen>" + Utils::GetDescription(gm));
	assert(token.size() > 1);
	if (token.size() == 2)
	{
		if (token[1] == "help" || token[1] == "h" || token[1] == "帮助")
		{
			logging::INFO("帮助文档 <Choyen>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#choyen [line1] [line2]"));
			return true;
		}
		else
		{
			logging::INFO("缺少参数[line2] <Choyen>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("你的第二行字捏"));
			return false;
		}
	}
	
	assert(token.size() > 2);
	if (token[1].empty())
	{
		logging::INFO("参数1为空 <Choyen>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("看不到第一句话捏，是口口剑22嘛"));
		return false;
	}
	if (token[2].empty())
	{
		logging::INFO("参数2为空 <Choyen>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("看不到第二句话捏，是口口剑22嘛"));
		return false;
	}

	const string url_local = Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000");
	httplib_ssl_zlib::Client cli(url_local);
	auto result = cli.Get("/gen/choyen/", {{"upper", token[1]}, {"lower", token[2]}}, {{"Accept-Encoding", "gzip"}});
	if (!Utils::CheckHttpResponse(result, "Choyen"))
	{
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	json msg = json::parse(result->body);

	assert(msg.contains("result"));
	logging::INFO("上传图片 <Choyen>" + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Image({"", "", "", msg["result"]}));
	return true;
}