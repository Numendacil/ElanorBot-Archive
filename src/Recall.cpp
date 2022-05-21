#include "utils/log.h"
#include "Command/Recall.hpp"
#include "Common.hpp"
#include "ElanorBot.hpp"
#include <mirai/messages/messages.hpp>
#include <mirai/exceptions/exceptions.hpp>

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

bool Recall::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Common::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#撤回"))
	{
		Common::ToLower(str);
		Common::Tokenize(token, str);
		if (token[0] == "#recall" || token[0] == "#撤回")
			return true;
	}
	return false;
}

bool Recall::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	logging::INFO("Calling Recall <Recall>" + Common::GetDescription(gm));
	if (token.size() > 1)
	{
		if (token[1] == "help" || token[1] == "h" || token[1] == "帮助")
		{
			logging::INFO("帮助文档 <Recall>" + Common::GetDescription(gm, false));
			Common::SendGroupMessage(gm, MessageChain().Plain("usage:\n#Recall [回复消息]"));
			return true;
		}
	}
	
	string url = "";
	auto quote = gm.MessageChain.GetAll<QuoteMessage>();
	if (quote.empty())
	{
		logging::INFO("格式错误 <Recall>: 未附带回复" + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("撤回啥捏"));
		return false;
	}
	try
	{
		this_thread::sleep_for(chrono::seconds(1));
		bot->client->Recall(quote[0].MessageId());
	}
	catch (MiraiApiHttpException& e)
	{
		logging::INFO("撤回失败 <Recall>: " + e.Message + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("撤回不了捏"));
		return false;
	}
	return false;
}