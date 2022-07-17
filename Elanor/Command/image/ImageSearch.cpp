#include <ThirdParty/log.h>
#include <ThirdParty/json.hpp>
#include <ThirdParty/httplib.hpp>
#include <State/CoolDown.hpp>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include <mirai.h>

#include "ImageSearch.hpp"

using namespace std;
using json = nlohmann::json;

namespace GroupCommand
{

bool ImageSearch::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#search"))
	{
		Utils::ToLower(str);
		Utils::Tokenize(tokens, str);
		if (tokens[0] == "#搜图" || tokens[0] == "#search")
			return true;
	}
	return false;
}

bool ImageSearch::Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const vector<string>& tokens) 
{
	logging::INFO("Calling ImageSearch <ImageSearch>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	enum {SAUCENAO, ASCII2D, TRACEMOE} server = SAUCENAO;
	if (tokens.size() > 1)
	{
		if (tokens[1] == "help" || tokens[1] == "h" || tokens[1] == "帮助")
		{
			logging::INFO("帮助文档 <ImageSearch>" + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("usage:\n#ImageSearch (server) [图片]"));
			return true;
		}

		else if (tokens[1] == "saucenao" || tokens[1] == "sauce")
		{
			server = SAUCENAO;
		}

		else if (tokens[1] == "ascii2d" || tokens[1] == "ascii")
		{
			server = ASCII2D;
		}

		else if (tokens[1] == "trace" || tokens[1] == "moe" || tokens[1] == "tracemoe")
		{
			server = TRACEMOE;
		}

		else
		{
			logging::INFO("未知搜索引擎 <ImageSearch>: " + tokens[1] + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain(tokens[1] + "是什么搜索引擎捏，不知道捏"));
			return false;
		}
	}
	
	logging::INFO("搜索引擎 <ImageSearch>: " + to_string(server) + Utils::GetDescription(gm, false));
	string url = "";
	auto img = gm.MessageChain.GetAll<Cyan::ImageMessage>();
	if (img.size())
	{
		url = img[0].ToMiraiImage().Url;
	}
	else
	{
		auto quote = gm.MessageChain.GetAll<Cyan::QuoteMessage>();
		if (quote.empty())
		{
			logging::INFO("格式错误 <ImageSearch>: 未附带图片或回复" + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("图捏"));
			return false;
		}
		try
		{
			Cyan::GroupMessage quote_gm = client.Call(&Cyan::MiraiBot::GetGroupMessageFromId, quote[0].MessageId());
			img = quote_gm.MessageChain.GetAll<Cyan::ImageMessage>();
			if (img.size())
			{
				url = img[0].ToMiraiImage().Url;
			}
			else
			{
				logging::INFO("格式错误 <ImageSearch>: 回复内容不包含图片" + Utils::GetDescription(gm, false));
				client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("图捏"));
				return false;
			}
		}
		catch (Cyan::MiraiApiHttpException &)
		{
			logging::INFO("无法从回复内容获得图片 <ImageSearch>" + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("消息太久了，看不到是什么图捏"));
			return false;
		}
	}
	logging::INFO("图片链接 <ImageSearch>: " + url);
	
	using namespace date;
	using namespace std::chrono;

	chrono::seconds cooldown;
	string path;

	switch(server)
	{
	case SAUCENAO:
		cooldown = 20s;
		path = "saucenao/";
		break;
	case ASCII2D:
		cooldown = 60s;
		path = "ascii2d/";
		break;
	case TRACEMOE:
		cooldown = 60s;
		path = "trace-moe/";
		break;

	default:	// You should never be here
		logging::ERROR("waht");
		return false;
	}


	auto cd = group.GetState<State::CoolDown>("CoolDown");
	chrono::seconds remaining;
	auto holder = cd->GetRemaining("ImageSearch", cooldown, remaining);

	if (remaining > 0s)
	{
		stringstream ss;
		ss << remaining;
		logging::INFO("冷却剩余 <ImageSearch>: " + ss.str() + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("冷却中捏（剩余: " + ss.str() + "）"));
		return false;
	}
	assert(holder);

	const string url_local = Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000");
	httplib_ssl_zlib::Client cli( url_local);
	auto result = cli.Get(("/image-search/" + path).c_str(), {{"url", url}}, {{"Accept-Encoding", "gzip"}});
	if (!Utils::CheckHttpResponse(result, "ImageSearch"))
	{
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	json msg = json::parse(result->body);

	assert(msg.contains("info"));
	Cyan::MessageChain message;
	message += msg["info"];
	if (msg.contains("image"))
		message = message + Cyan::MessageChain().Image({.Base64 = msg["image"]});
	logging::INFO("上传结果 <ImageSearch>" + Utils::GetDescription(gm, false));
	client.Send(gm.Sender.Group.GID, message);
	return true;
}

}