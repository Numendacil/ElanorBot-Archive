#include "Command/pjsk/pjskChart.hpp"
#include "third-party/json.hpp"
#include "third-party/date.h"
#include "State/CoolDown.hpp"
#include "ElanorBot.hpp"
#include "Utils.hpp"


using namespace std;
using namespace Cyan;
using namespace date;
using json = nlohmann::json;

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

	vector<pair<json, unordered_set<string>>> alias_pair;
	{
		ifstream ifile(Utils::MediaFilePath + "music/pjsk/alias.json");
		json alias = json::parse(ifile);
		ifile.close();

		for (const auto &p : alias.items())
		{
			alias_pair.emplace_back(p.value(), p.value()["alias"]);
		}
	}

	json music;
	for (const auto& p : alias_pair)
	{
		if (p.second.contains(target))
		{
			music = p.first;
			break;
		}
	}

	if (music.is_null())
	{
		logging::INFO("未知歌曲 <pjskChart>: " + target + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain(target + "是什么歌捏，不知道捏"));
		return false;
	}
	logging::INFO("获取歌曲谱面 <pjskChart>: " + music["title"].get<string>() + Utils::GetDescription(gm, false));
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
	string url = "https://minio.dnaroma.eu/sekai-music-charts/" + string(id) + "/" + difficulty + ".png";
	Utils::SendGroupMessage(gm, MessageChain().Plain("谱面: " + music["title"].get<string>() 
							+ "\n难度: " + difficulty
							+ "\n")
							.Image({"", url, "", ""}));
	return true;
}