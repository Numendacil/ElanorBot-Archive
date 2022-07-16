#include <Trigger/BililiveTrigger.hpp>
#include <State/BililiveList.hpp>
#include <app/ElanorBot.hpp>
#include <Utils/MessageQueue.hpp>
#include <ThirdParty/httplib.hpp>
#include <ThirdParty/log.h>
#include <ThirdParty/json.hpp>
#include <Utils/Utils.hpp>

using namespace std;
;
using json = nlohmann::json;

void BililiveTrigger::Action(shared_ptr<MiraiBot> client, ElanorBot* bot)
{
	assert(bot);
	auto BiliList = bot->GetState<BililiveList>("BililiveList");
	auto GroupInfo = client->GetGroupConfig(bot->gid);
	httplib_ssl_zlib::Client cli("https://api.live.bilibili.com");
	for (const auto& id : BiliList->GetList())
	{
		auto result = cli.Get("/room/v1/Room/get_info", {{"id", to_string(id.second.room_id)}}, 
					{{"Accept-Encoding", "gzip"},
					{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
		if (!Utils::CheckHttpResponse(result, "BililiveTrigger: room_info"))
			continue;

		json content = json::parse(result->body);
		if (content["code"].get<int>() != 0)
		{
			logging::WARN("Error response from /room/v1/Room/get_info <BililiveTrigger>: " + content["msg"].get<string>());
			continue;
		}
		if (content["data"]["live_status"].get<int>() == 1)
		{
			if (id.second.broadcasted)
				continue;
			string title = content["data"]["title"].get<string>();
			string cover = content["data"]["user_cover"].get<string>();
			string area = content["data"]["area_name"].get<string>();
			result = cli.Get("/live_user/v1/Master/info", {{"uid", to_string(id.first)}}, 
					{{"Accept-Encoding", "gzip"},
					{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
			if (!Utils::CheckHttpResponse(result, "BililiveTrigger: user_info"))
				continue;

			content = json::parse(result->body);
			if (content["code"].get<int>() != 0)
			{
				logging::WARN("Error response from /live_user/v1/Master/info <BililiveTrigger>: " + content["msg"].get<string>());
				continue;
			}
			BiliList->Broadcast(id.first);
			string message = content["data"]["info"]["uname"].get<string>() 
				+ " (" + to_string(id.first) + ") 正在直播: " + title;
			MessageChain msg = MessageChain().Plain(message)
				.Plain("\n分区: " + area + "\n")
				.Image({"", cover, "", ""})
				.Plain("\nhttps://live.bilibili.com/" + to_string(id.second.room_id));
			logging::INFO("发送开播信息 <BililiveTrigger>: " + message + "\t-> " + GroupInfo.Name + "(" + to_string(bot->gid.ToInt64()) + ")");
			MessageQueue::GetInstance().Push(bot->gid, msg);
		}
		else
			BiliList->ResetBroadcast(id.first);

		this_thread::sleep_for(chrono::milliseconds(2000));
	}
}