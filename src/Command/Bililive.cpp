#include "Command/Bililive.hpp"
#include "State/BililiveList.hpp"
#include "Utils.hpp"
#include "third-party/log.h"
#include "third-party/httplib.hpp"
#include "third-party/json.hpp"
#include "ElanorBot.hpp"

using namespace std;
using namespace Cyan;
using json = nlohmann::json;


bool Bililive::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#live"))
	{
		if (Utils::Tokenize(token, str) < 2)
			return false;
		Utils::ToLower(token[0]);
		if (token[0] == "#live" || token[0] == "#直播")
			return true;
	}
	return false;
}

bool Bililive::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling Bililive <Bililive>" + Utils::GetDescription(gm));
	auto BiliList = bot->GetState<BililiveList>("BililiveList");
	string command = token[1];
	Utils::ToLower(command);


	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档 <Bililive>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#live add [uid]\n#live del [uid]\n#live list"));
		return true;
	}

	httplib_ssl_zlib::Client cli("https://api.live.bilibili.com");
	if (command == "list")
	{
		string message = "直播间列表: ";
		for (const auto &id : BiliList->GetList())
		{
			auto result = cli.Get("/live_user/v1/Master/info", {{"uid", to_string(id.first)}},
					      {{"Accept-Encoding", "gzip"},
					       {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
			if (!Utils::CheckHttpResponse(result, "Bililive: user_info"))
			{
				Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return false;
			}

			json content = json::parse(result->body);
			if (content["code"].get<int>() != 0)
			{
				logging::WARN("Error response from /live_user/v1/Master/info <Bililive>: " + content["msg"].get<string>());
				Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return false;
			}
			message += "\n" + content["data"]["info"]["uname"].get<string>() + " (" + to_string(id.first) + "): ";

			result = cli.Get("/room/v1/Room/get_info", {{"id", to_string(id.second.room_id)}},
					 {{"Accept-Encoding", "gzip"},
					  {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
			if (!Utils::CheckHttpResponse(result, "Bililive: room_info"))
			{
				Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return false;
			}

			content = json::parse(result->body);
			if (content["code"].get<int>() != 0)
			{
				logging::WARN("Error response from /room/v1/Room/get_info <Bililive>: " + content["msg"].get<string>());
				Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
				return false;
			}
			if (content["data"]["live_status"].get<int>() == 0)
				message += "未开播 ⚫";
			else
				message += (content["data"]["live_status"].get<int>() == 1) ? "直播中 🔴" : "轮播中 🔵";
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		logging::INFO("输出直播间列表 <Bililive>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain(message));
		return true;
	}

	if (command == "add" || command == "del")
	{
		if (token.size() < 3)
		{
			logging::INFO("缺少参数[uid] <Bililive>: " + command + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("缺少参数[uid]，是被你吃了嘛"));
			return false;
		}

		long uid;
		try
		{
			uid = stol(token[2]);
		}
		catch (const logic_error& e)
		{
			logging::INFO("无效参数[uid] <Bililive>: " + token[2] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(token[2] + "是个锤子uid"));
			return false;
		}

		auto result = cli.Get("/live_user/v1/Master/info'", {{"uid", to_string(uid)}}, 
				{{"Accept-Encoding", "gzip"},
				{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		if (!Utils::CheckHttpResponse(result, "Bililive: user_info"))
		{
			Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}

		json content = json::parse(result->body);
		if (content["code"].get<int>() != 0)
		{
			logging::WARN("Error response from /live_user/v1/Master/info' <Bililive>: " + content["msg"].get<string>());
			Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
			return false;
		}
		if (content["data"]["info"]["uname"].get<string>().empty())
		{
			logging::INFO("用户不存在 <Bililive>: " + token[2] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("该用户不存在捏"));
			return false;
		}
		if (content["data"]["room_id"].get<long>() == 0)
		{
			logging::INFO("直播间不存在 <Bililive>: " + token[2] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("该用户貌似暂未开通直播功能捏"));
			return false;
		}

		if (command == "add")
		{
			if (BiliList->Exist(uid))
			{
				logging::INFO("用户已存在 <Bililive>: " + token[2] + Utils::GetDescription(gm, false));
				Utils::SendGroupMessage(gm, MessageChain().Plain("该用户已经在名单里了捏"));
				return false;
			}
			long room_id = content["data"]["room_id"].get<long>();
			string pic = content["data"]["info"]["face"].get<string>();
			string name = content["data"]["info"]["uname"].get<string>();
			BiliList->Insert(uid, room_id);
			logging::INFO("成功添加用户 <Bililive>: " + name + "(" + to_string(uid) + ")" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain()
						.Plain("成功添加用户" + name + "(" + to_string(uid) + ")\n")
						.Image({"", pic, "", ""}));
			return true;
		}

		if (command == "del")
		{
			if (!BiliList->Exist(uid))
			{
				logging::INFO("用户不存在 <Bililive>: " + token[2] + Utils::GetDescription(gm, false));
				Utils::SendGroupMessage(gm, MessageChain().Plain("该用户还不在名单里捏"));
				return false;
			}
			string pic = content["data"]["info"]["face"].get<string>();
			string name = content["data"]["info"]["uname"].get<string>();
			BiliList->Erase(uid);
			logging::INFO("成功删除用户 <Bililive>: " + name + "(" + to_string(uid) + ")" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain()
						.Plain("成功删除用户" + name + "(" + to_string(uid) + ")\n")
						.Image({"", pic, "", ""}));
			return true;
		}
	}
	

	logging::INFO("未知命令 <Bililive>: " + token[1] + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Plain(token[1] + "是什么指令捏，不知道捏"));
	return false;
}