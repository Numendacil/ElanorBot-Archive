#include "third-party/log.h"
#include "third-party/json.hpp"
#include "third-party/httplib.hpp"
#include "State/CoolDown.hpp"
#include "Command/image/Pixiv.hpp"
#include "Utils.hpp"
#include "ElanorBot.hpp"
#include <mirai/messages/messages.hpp>
#include <mirai/exceptions/exceptions.hpp>

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

bool Pixiv::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() > char_traits<char>::length("#pix"))
	{
		if (Utils::Tokenize(token, str) < 2)
			return false;
		Utils::ToLower(token[0]);
		if (token[0] == "#pixiv" || token[0] == "#p站" || token[0] == "#pix")
			return true;
	}
	return false;
}

bool GetPixivById(const GroupMessage& gm, shared_ptr<ElanorBot> bot, long pid, int page)
{
	using namespace date;
	using namespace std::chrono;
	auto cd = bot->GetState<CoolDown>("CoolDown");
	chrono::seconds remaining;
	auto holder = cd->GetRemaining("Pixiv", 20s, remaining);

	if (remaining > 0s)
	{
		stringstream ss;
		ss << remaining;
		logging::INFO("冷却剩余<Pixiv>: " + ss.str() + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("冷却中捏（剩余: " + ss.str() + "）"));
		return false;
	}
	assert(holder);

	httplib_ssl_zlib::Client cli("localhost", 8000);
	auto result = cli.Get("/pixiv/id/", {{"id", to_string(pid)}, {"page", to_string(page)}}, {{"Accept-Encoding", "gzip"}});
	if (!Utils::CheckHttpResponse(result, "Pixiv"))
	{
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	json msg = json::parse(result->body);

	assert(msg.contains("info"));
	if (!msg.contains("image"))
	{
		logging::INFO("上传结果<Pixiv>: 未找到图片" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain(msg["info"].get<string>()));
		return true;
	}
	json info = json::parse(msg["info"].get<string>());
	string message;
	try
	{
		message = "标题: " + info["title"].get<string>() + " (id: " + to_string(info["id"]) + ")\n作者: " + info["user"]["name"].get<string>() + " (id: " + to_string(info["user"]["id"]) + ")\n标签:";
		for (auto p = info["tags"].begin(); p < info["tags"].end() && p < info["tags"].begin() + 7; p++)
		{
			if (!(*p)["translated_name"].empty() && (*p)["name"].get<string>().find("users入り") == string::npos)
			{
				message += " #" + (*p)["name"].get<string>() + " (" + (*p)["translated_name"].get<string>() + ")  ";
			}
			else
			{
				message += " #" + (*p)["name"].get<string>() + "  ";
			}
		}
		if (info["tags"].size() > 7)
		{
			message += " ...";
		}
		message += '\n';
		int num_page = info["page_count"].get<int>();
		if (num_page > 1)
		{
			message += "页码: " + to_string((num_page > page - 1) ? page : 1) + "/" + to_string(num_page) + "\n";
		}
	}
	catch (const exception& e)
	{
		logging::WARN("Error occured while parsing result <Pixiv>: " +string(e.what()) + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}

	logging::INFO("上传结果 <Pixiv>" + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Plain(message).Image({"", "", "", msg["image"]}));
	return true;
}

bool Pixiv::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	logging::INFO("Calling Pixiv<Pixiv>" + Utils::GetDescription(gm));
	assert(token.size() > 1);
	string command = token[1];
	Utils::ToLower(command);
	if (command == "help" || command == "h" || command == "帮助")
	{
		logging::INFO("帮助文档<Pixiv>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("usage:\n#pixiv id [pid] (page)"));
		return true;
	}
	else if (command == "id")
	{
		if (token.size() < 3)
		{
			logging::INFO("缺少参数[pid] <Pixiv>" + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain("不发pid看个锤子图"));
			return true;
		}
		long pid = 0;
		int page = 1;
		try
		{
			pid = stol(token[2]);
			if (pid <= 0)
				throw logic_error("[pid] must be positive");
		}
		catch (const logic_error& e)
		{
			logging::INFO("无效[pid] <Pixiv>: " + token[2] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(token[2] + "是个锤子pid"));
			return false;
		}
		if (token.size() > 3)
		{
			try
			{
				page = stoi(token[3]);
				if (page <= 0)
					throw logic_error("[page] must be positive");
			}
			catch (const logic_error &e)
			{
				logging::INFO("无效[page] <Pixiv>: " + token[3] + Utils::GetDescription(gm, false));
				Utils::SendGroupMessage(gm, MessageChain().Plain(token[3] + "是个锤子页码"));
				return false;
			}
		}
		return GetPixivById(gm, bot, pid, page);
	}

	logging::INFO("未知指令 <Pixiv>: " + token[1] + Utils::GetDescription(gm, false));
	Utils::SendGroupMessage(gm, MessageChain().Plain(token[1] + "是什么指令捏，不认识捏"));
	return false;
}