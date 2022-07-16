#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <mirai/messages/messages.hpp>
#include <mirai/exceptions/exceptions.hpp>

#include "Recall.hpp"
#include "mirai/MiraiBot.hpp"

using namespace std;
using json = nlohmann::json;

namespace GroupCommand
{

bool Recall::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#撤回"))
	{
		Utils::ToLower(str);
		Utils::Tokenize(tokens, str);
		if (tokens[0] == "#recall" || tokens[0] == "#撤回")
			return true;
	}
	return false;
}

bool Recall::Execute(const Cyan::GroupMessage& gm, Group& group, const vector<string>& tokens) 
{
	Client& client = Client::GetClient();
	logging::INFO("Calling Recall <Recall>" + Utils::GetDescription(gm));
	if (tokens.size() > 1)
	{
		if (tokens[1] == "help" || tokens[1] == "h" || tokens[1] == "帮助")
		{
			logging::INFO("帮助文档 <Recall>" + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("usage:\n#Recall [回复消息]"));
			return true;
		}
	}
	
	string url = "";
	auto quote = gm.MessageChain.GetAll<Cyan::QuoteMessage>();
	if (quote.empty())
	{
		logging::INFO("格式错误 <Recall>: 未附带回复" + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("撤回啥捏"));
		return false;
	}
	try
	{
		this_thread::sleep_for(chrono::seconds(1));
		client.Call(&Cyan::MiraiBot::Recall, quote[0].MessageId());
	}
	catch (Cyan::MiraiApiHttpException& e)
	{
		logging::INFO("撤回失败 <Recall>: " + e.Message + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("撤回不了捏"));
		return false;
	}
	return false;
}

}