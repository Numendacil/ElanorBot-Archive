#include <third-party/log.h>
#include <third-party/json.hpp>
#include <third-party/httplib.hpp>
#include <State/CoolDown.hpp>
#include <Command/image/ImageSearch.hpp>
#include <Utils/Utils.hpp>
#include <app/ElanorBot.hpp>
#include <mirai/messages/messages.hpp>
#include <mirai/exceptions/exceptions.hpp>

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

bool ImageSearch::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#search"))
	{
		Utils::ToLower(str);
		Utils::Tokenize(token, str);
		if (token[0] == "#搜图" || token[0] == "#search")
			return true;
	}
	return false;
}

bool ImageSearch::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	logging::INFO("Calling ImageSearch <ImageSearch>" + Utils::GetDescription(gm));
	enum IMG_SERVER {SAUCENAO, ASCII2D};
	IMG_SERVER server = SAUCENAO;
	if (token.size() > 1)
	{
		if (token[1] == "help" || token[1] == "h" || token[1] == "帮助")
		{
			logging::INFO("帮助文档 <ImageSearch>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#ImageSearch (server) [图片]"));
			return true;
		}

		else if (token[1] == "saucenao" || token[1] == "sauce")
		{
			server = SAUCENAO;
		}

		else if (token[1] == "ascii2d" || token[1] == "ascii")
		{
			server = ASCII2D;
		}

		else
		{
			logging::INFO("未知搜索引擎 <ImageSearch>: " + token[1] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(token[1] + "是什么搜索引擎捏，不知道捏"));
			return false;
		}
	}
	
	logging::INFO("搜索引擎 <ImageSearch>: " + to_string(server) + Utils::GetDescription(gm, false));
	string url = "";
	auto img = gm.MessageChain.GetAll<ImageMessage>();
	if (img.size())
	{
		url = img[0].ToMiraiImage().Url;
	}
	else
	{
		auto quote = gm.MessageChain.GetAll<QuoteMessage>();
		if (quote.empty())
		{
			logging::INFO("格式错误 <ImageSearch>: 未附带图片或回复" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("图捏"));
			return false;
		}
		try
		{
			GroupMessage quote_gm = bot->client->GetGroupMessageFromId(quote[0].MessageId());
			img = quote_gm.MessageChain.GetAll<ImageMessage>();
			if (img.size())
			{
				url = img[0].ToMiraiImage().Url;
			}
			else
			{
				logging::INFO("格式错误 <ImageSearch>: 回复内容不包含图片" + Utils::GetDescription(gm, false));
				Utils::SendGroupMessage(gm, MessageChain().Plain("图捏"));
				return false;
			}
		}
		catch (MiraiApiHttpException &)
		{
			logging::INFO("无法从回复内容获得图片 <ImageSearch>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("消息太久了，看不到是什么图捏"));
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

	default:	// You should never be here
		logging::ERROR("waht");
		return false;
	}


	auto cd = bot->GetState<CoolDown>("CoolDown");
	chrono::seconds remaining;
	auto holder = cd->GetRemaining("ImageSearch", cooldown, remaining);

	if (remaining > 0s)
	{
		stringstream ss;
		ss << remaining;
		logging::INFO("冷却剩余 <ImageSearch>: " + ss.str() + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("冷却中捏（剩余: " + ss.str() + "）"));
		return false;
	}
	assert(holder);

	const string url_local = Utils::Configs.Get<string>("/PythonServer"_json_pointer, "localhost:8000");
	httplib_ssl_zlib::Client cli( url_local);
	auto result = cli.Get(("/image-search/" + path).c_str(), {{"url", url}}, {{"Accept-Encoding", "gzip"}});
	if (!Utils::CheckHttpResponse(result, "ImageSearch"))
	{
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	json msg = json::parse(result->body);

	assert(msg.contains("info"));
	MessageChain message;
	message += msg["info"];
	if (msg.contains("image"))
		message = message + MessageChain().Image({"", "", "", msg["image"]});
	logging::INFO("上传结果 <ImageSearch>" + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, message);
	return true;
}