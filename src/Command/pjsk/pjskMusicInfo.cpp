#include "Command/pjsk/pjskMusicInfo.hpp"
#include "third-party/json.hpp"
#include "third-party/date.h"
#include "State/CoolDown.hpp"
#include "ElanorBot.hpp"
#include "Utils.hpp"


using namespace std;
using namespace Cyan;
using namespace date;
using json = nlohmann::json;



bool pjskMusicInfo::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk info"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(token, str) < 3)
			return false;
		if (token[0] == "#pjsk" || token[0] == "#啤酒烧烤" || token[0] == "#prsk")
			if (token[1] == "info" || token[1] == "歌曲信息")
			return true;
	}
	return false;
}



bool pjskMusicInfo::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 2);
	logging::INFO("Calling pjskMusicInfo <pjskMusicInfo>" + Utils::GetDescription(gm));
	string target = token[2];
	const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");

	json music;
	{
		vector<pair<json, unordered_set<string>>> alias_pair;
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
		logging::INFO("未知歌曲 <pjskMusicInfo>: " + target + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain(target + "是什么歌捏，不知道捏"));
		return false;
	}
	int id = music["musicId"].get<int>();
	vector<string> alias =  music["alias"].get<vector<string>>();
	string title = music["title"].get<string>();
	string translate = music["translate"].get<string>();
	double length = -1;
	string cover_path;
	logging::INFO("获取歌曲信息 <pjskMusicInfo>: " + title + Utils::GetDescription(gm, false));
	{
		ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
		json meta = json::parse(ifile);
		ifile.close();

		for (const auto &p : meta.items())
		{
			if (p.value()["musicId"].get<int>() == id)
			{
				length = p.value()["length"].get<double>();
				cover_path = MediaFilesPath
						+ "images/pjsk/cover_small/" + p.value()["assetbundleName"].get<string>() + "_small.png";;
			}
		}
	}
	assert(length > 0);
	string msg = 	"歌曲id: " + to_string(id)
			+ "\n曲名: " + title
			+ "\n译名: " + translate
			+ "\n时长: " + to_string(int(length))
			+ "s\n其它名称:";
	for (const auto& s : alias)
	{
		msg += " 「" + s + "」";
	}
	msg += '\n';
	Utils::SendGroupMessage(gm, MessageChain().Plain(msg).Image({"", "", cover_path, ""}));
	
	return true;
}