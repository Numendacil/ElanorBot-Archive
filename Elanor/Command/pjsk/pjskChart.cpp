#include <ThirdParty/json.hpp>
#include <ThirdParty/date.h>
#include <ThirdParty/httplib.hpp>
#include <ThirdParty/cppcodec/base64_rfc4648.hpp>
#include <State/CoolDown.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <Utils/Utils.hpp>

#include "pjskChart.hpp"

using namespace std;
using namespace date;
using json = nlohmann::json;
using base64 = cppcodec::base64_rfc4648;

namespace GroupCommand
{

bool pjskChart::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk chart"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(tokens, str) < 3)
			return false;
		if (tokens[0] == "#pjsk" || tokens[0] == "#啤酒烧烤" || tokens[0] == "#prsk")
			if (tokens[1] == "chart" || tokens[1] == "谱面")
			return true;
	}
	return false;
}



bool pjskChart::Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const vector<string>& tokens) 
{
	assert(tokens.size() > 2);
	logging::INFO("Calling pjskChart <pjskChart>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();
	string target = tokens[2];

	json music;
	{
		vector<pair<json, unordered_set<string>>> alias_pair;
		const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
		ifstream ifile(MediaFilesPath + "music/pjsk/alias.json");

		if (!ifile)
		{
			logging::WARN("Unable to open alias.json <pjskChart>");
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain(target + "是什么歌捏，不知道捏"));
			return false;
		}

		json alias = json::parse(ifile);
		ifile.close();

		for (const auto &p : alias.items())
		{
			unordered_set<string> s;
			for (const auto &item : p.value()["alias"].items())
			{
				string str = string(item.value());
				Utils::ToLower(str);
				Utils::ReplaceMark(str);
				s.insert(str);
			}
			alias_pair.emplace_back(p.value(), s);
		}
		for (const auto &p : alias_pair)
		{
			if (p.second.contains(target))
			{
				music = p.first;
				break;
			}
		}
	}

	if (music.is_null())
	{
		logging::INFO("未知歌曲 <pjskChart>: " + target + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain(target + "是什么歌捏，不知道捏"));
		return false;
	}
	logging::INFO("获取歌曲谱面 <pjskChart>: " + music["title"].get<string>());
	string difficulty = "master";
	if (tokens.size() > 3)
	{
		if (tokens[3] == "easy" || tokens[3] == "ez" || tokens[3] == "简单")		difficulty = "easy";
		else if (tokens[3] == "normal" || tokens[3] == "nm" || tokens[3] == "一般")	difficulty = "normal";
		else if (tokens[3] == "hard" || tokens[3] == "hd" || tokens[3] == "困难")		difficulty = "hard";
		else if (tokens[3] == "expert" || tokens[3] == "ex" || tokens[3] == "专家")	difficulty = "expert";
		else if (tokens[3] == "master" || tokens[3] == "ma" || tokens[3] == "大师")	difficulty = "master";
		else
		{
			logging::INFO("未知难度 <pjskChart>: " + tokens[3] + Utils::GetDescription(gm, false));
			client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain(tokens[3] + "是什么难度捏，不知道捏"));
			return false;
		}
	}

	auto cd = group.GetState<State::CoolDown>();
	chrono::seconds remaining;
	auto holder = cd->GetRemaining("pjskChart", 20s, remaining);

	if (remaining > 0s)
	{
		stringstream ss;
		ss << remaining;
		logging::INFO("冷却剩余 <pjskChart>: " + ss.str() + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("冷却中捏（剩余: " + ss.str() + "）"));
		return false;
	}
	assert(holder);

	char id[10];
	assert(music["musicId"].get<int>() < 10000);
	sprintf(id, "%04d", music["musicId"].get<int>());
	httplib_ssl_zlib::Client resource_cli("https://sekai-music-charts-1258184166.file.myqcloud.com");
	auto resp = resource_cli.Get(("/" + string(id) + "/" + difficulty + ".png").c_str(),
				     {{"Accept-Encoding", "gzip"},
				      {"Referer", "https://sekai.best/"},
				      {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
	if (!Utils::CheckHttpResponse(resp, "pjskChart: " + string(id) + " " + difficulty))
	{
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	logging::INFO("图片下载完成 <pjskChart>");
	client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("谱面: " + music["title"].get<string>() 
							+ "\n难度: " + difficulty
							+ "\n")
							.Image({.Base64 = base64::encode(resp->body)}));
	return true;
}

}