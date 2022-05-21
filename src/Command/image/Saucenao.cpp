#include "utils/log.h"
#include "utils/json.hpp"
#include "utils/httplib.hpp"
#include "State/CoolDown.hpp"
#include "Command/image/Saucenao.hpp"
#include "Common.hpp"
#include "ElanorBot.hpp"
#include <mirai/messages/messages.hpp>
#include <mirai/exceptions/exceptions.hpp>

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

bool Saucenao::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Common::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#sauce"))
	{
		Common::ToLower(str);
		Common::Tokenize(token, str);
		if (token[0] == "#sauce" || token[0] == "#搜图" || token[0] == "#saucenao")
			return true;
	}
	return false;
}

bool Saucenao::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	logging::INFO("Calling Saucenao <Saucenao>" + Common::GetDescription(gm));
	if (token.size() > 1)
	{
		if (token[1] == "help" || token[1] == "h" || token[1] == "帮助")
		{
			logging::INFO("帮助文档 <Saucenao>" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("usage:\n#saucenao [图片]"));
			return true;
		}
	}
	
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
			logging::INFO("格式错误 <Saucenao>: 未附带图片或回复" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("图捏"));
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
				logging::INFO("格式错误 <Saucenao>: 回复内容不包含图片" + Common::GetDescription(gm, false));
				Common::SendGroupMessage(gm, MessageChain().Plain("图捏"));
				return false;
			}
		}
		catch (MiraiApiHttpException &)
		{
			logging::INFO("无法从回复内容获得图片 <Saucenao>" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("消息太久了，看不到是什么图捏"));
			return false;
		}
	}
	logging::INFO("图片链接 <Saucenao>: " + url);
	
	using namespace date;
	using namespace std::chrono;
	auto cd = bot->GetState<CoolDown>("CoolDown");
	chrono::seconds remaining;
	auto holder = cd->GetRemaining("Saucenao", 20s, remaining);

	if (remaining > 0s)
	{
		stringstream ss;
		ss << remaining;
		logging::INFO("冷却剩余 <Saucenao>: " + ss.str() + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("冷却中捏（剩余: " + ss.str() + "）"));
		return false;
	}
	assert(holder);

	httplib_ssl_zlib::Client cli("localhost", 8000);
	auto result = cli.Get("/image/saucenao/", {{"url", url}}, {{"Accept-Encoding", "gzip"}});
	if (!Common::CheckHttpResponse(result, "Pixiv"))
	{
		Common::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	json msg = json::parse(result->body);

	assert(msg.contains("info"));
	MessageChain message;
	message += msg["info"];
	if (msg.contains("image"))
		message = message + MessageChain().Image({"", "", "", msg["image"]});
	logging::INFO("上传结果 <Saucenao>" + Common::GetDescription(gm, false));
	Common::SendGroupMessage(gm, message);
	return true;
}