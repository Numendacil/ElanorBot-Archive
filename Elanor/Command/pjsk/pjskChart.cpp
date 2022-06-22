#include <Command/pjsk/pjskChart.hpp>
#include <third-party/json.hpp>
#include <third-party/date.h>
#include <third-party/httplib.hpp>
#include <third-party/cppcodec/base64_rfc4648.hpp>
#include <State/CoolDown.hpp>
#include <app/ElanorBot.hpp>
#include <Utils/Utils.hpp>


using namespace std;
using namespace Cyan;
using namespace date;
using namespace httplib_ssl_zlib;
using json = nlohmann::json;
using base64 = cppcodec::base64_rfc4648;

bool pjskChart::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk chart"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(token, str) < 3)
			return false;
		if (token[0] == "#pjsk" || token[0] == "#啤酒烧烤" || token[0] == "#prsk")
			if (token[1] == "chart" || token[1] == "谱面")
			return true;
	}
	return false;
}



bool pjskChart::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 2);
	logging::INFO("Calling pjskChart <pjskChart>" + Utils::GetDescription(gm));
	string target = token[2];

	json music;
	{
		vector<pair<json, unordered_set<string>>> alias_pair;
		const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
		ifstream ifile(MediaFilesPath + "music/pjsk/alias.json");
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
		Utils::SendGroupMessage(gm, MessageChain().Plain(target + "是什么歌捏，不知道捏"));
		return false;
	}
	logging::INFO("获取歌曲谱面 <pjskChart>: " + music["title"].get<string>());
	string difficulty = "master";
	if (token.size() > 3)
	{
		if (token[3] == "easy" || token[3] == "ez" || token[3] == "简单")		difficulty = "easy";
		else if (token[3] == "normal" || token[3] == "nm" || token[3] == "一般")	difficulty = "normal";
		else if (token[3] == "hard" || token[3] == "hd" || token[3] == "困难")		difficulty = "hard";
		else if (token[3] == "expert" || token[3] == "ex" || token[3] == "专家")	difficulty = "expert";
		else if (token[3] == "master" || token[3] == "ma" || token[3] == "大师")	difficulty = "master";
		else
		{
			logging::INFO("未知难度 <pjskChart>: " + token[3] + Utils::GetDescription(gm, false));
			Utils::SendGroupMessage(gm, MessageChain().Plain(token[3] + "是什么难度捏，不知道捏"));
			return false;
		}
	}

	auto cd = bot->GetState<CoolDown>("CoolDown");
	chrono::seconds remaining;
	auto holder = cd->GetRemaining("pjskChart", 20s, remaining);

	if (remaining > 0s)
	{
		stringstream ss;
		ss << remaining;
		logging::INFO("冷却剩余 <pjskChart>: " + ss.str() + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("冷却中捏（剩余: " + ss.str() + "）"));
		return false;
	}
	assert(holder);

	char id[10];
	assert(music["musicId"].get<int>() < 10000);
	sprintf(id, "%04d", music["musicId"].get<int>());
	Client resource_cli("https://sekai-music-charts-1258184166.file.myqcloud.com");
	auto resp = resource_cli.Get(("/" + string(id) + "/" + difficulty + ".png").c_str(),
				     {{"Accept-Encoding", "gzip"},
				      {"Referer", "https://sekai.best/"},
				      {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0"}});
	if (!Utils::CheckHttpResponse(resp, "pjskChart: " + string(id) + " " + difficulty))
	{
		Utils::SendGroupMessage(gm, MessageChain().Plain("该服务寄了捏，怎么会事捏"));
		return false;
	}
	logging::INFO("图片下载完成 <pjskChart>");
	Utils::SendGroupMessage(gm, MessageChain().Plain("谱面: " + music["title"].get<string>() 
							+ "\n难度: " + difficulty
							+ "\n")
							.Image({"", "", "", base64::encode(resp->body)}));
	return true;
}