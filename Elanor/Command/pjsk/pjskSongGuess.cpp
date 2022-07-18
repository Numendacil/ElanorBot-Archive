#include <State/Activity.hpp>
#include <ThirdParty/log.h>
#include <ThirdParty/uuid.h>
#include <ThirdParty/json.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <Utils/Utils.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

#include <mirai.h>

#include "pjskSongGuess.hpp"

using namespace std;
using json = nlohmann::json;

namespace GroupCommand
{

bool pjskSongGuess::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk 猜歌"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(tokens, str) < 2)
			return false;
		if (tokens[0] == "#pjsk" || tokens[0] == "#啤酒烧烤" || tokens[0] == "#prsk")
			if (tokens[1] == "猜歌")
				return true;
	}
	return false;
}



bool pjskSongGuess::Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const vector<string>& tokens) 
{
	assert(tokens.size() > 1);
	logging::INFO("Calling pjskSongGuess <pjskSongGuess>" + Utils::GetDescription(gm));
	Bot::Client& client = Bot::Client::GetClient();

	auto state = group.GetState<State::Activity>();
	auto holder = state->CheckAndStart("pjsk");
	const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
	if (!holder)
	{
		logging::INFO("有活动正在进行 <pjskSongGuess>" + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("有活动正在进行中捏"));
		return false;
	}

	client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("请在规定时间内发送音频选段对应的歌曲名称。回答请以句号开头捏"));

	json music, alias;
	{
		ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
		json meta_data = json::parse(ifile);
		ifile.close();

		uniform_int_distribution<int> rng1(0, meta_data.size() - 1);
		do
		{
			music = meta_data[rng1(Utils::rng_engine)];
			uniform_int_distribution<int> rng2(0, music["vocal"].size() - 1);
			music["vocal"] = music["vocal"][rng2(Utils::rng_engine)];
		} 
		while (!filesystem::exists(MediaFilesPath + "music/pjsk/songs/" + music["vocal"]["assetbundleName"].get<string>() + ".mp3"));
	}
	{
		ifstream ifile(MediaFilesPath + "music/pjsk/alias.json");
		json alias_data = json::parse(ifile);
		ifile.close();

		for (const auto& p : alias_data.items())
		{
			if (p.value()["musicId"] == music["musicId"])
			{
				alias = p.value()["alias"];
				break;
			}
		}
	}
	logging::INFO("题目为 <pjskSongGuess>: " + music.dump() + alias.dump());


	unordered_set<string> alias_map;
	for (const auto &item : alias.items())
	{
		string str = string(item.value());
		Utils::ToLower(str);
		Utils::ReplaceMark(str);
		alias_map.insert(str);
	}


	string music_path, audio_ans_path;
	double length, pos;
	const int round = Utils::Configs.Get<int>("/pjsk/SongGuess/Round"_json_pointer, 2);
	const double interval = Utils::Configs.Get<double>("/pjsk/SongGuess/Interval"_json_pointer, 1.0);
	const double incr = Utils::Configs.Get<double>("/pjsk/SongGuess/IncrRate"_json_pointer, 0.8);
	{
		music_path = MediaFilesPath + "music/pjsk/songs/" + music["vocal"]["assetbundleName"].get<string>() + ".mp3";
		uuids::basic_uuid_random_generator rng(Utils::rng_engine);
		string tmp1 = MediaFilesPath + "tmp/" + to_string(rng()) + ".mp3";
		string tmp2 = MediaFilesPath + "tmp/" + to_string(rng()) + ".pcm";
		audio_ans_path = MediaFilesPath + "tmp/" + to_string(rng()) + ".slk";

		if (music.contains("length"))
			length = music["length"].get<double>();
		else
		{
			string result = Utils::exec({
							"ffprobe",
							"-i", music_path,
							"-show_entries", "format=duration",
							"-v", "quiet",
							"-of", "csv=p=0"});
			length = stod(result);
		}
		double max_inter_2 = interval * ((round - 1) * 0.8 + 1.0) / 2.0;
		double ans_interval_2 = 4;
		uniform_real_distribution<double> rng_real(2.0 + ans_interval_2 + max_inter_2, length - max_inter_2 - 2.0 - ans_interval_2);
		pos = rng_real(Utils::rng_engine);

		Utils::exec({
			"ffmpeg",
			"-ss", to_string(pos - ans_interval_2 - max_inter_2),
			"-t", to_string((max_inter_2 + ans_interval_2) * 2.0),
			"-v", "quiet",
			"-i", music_path, 
			"-acodec", "copy", 
			tmp1
		});

		Utils::exec({
			"ffmpeg", 
			"-i", tmp1, 
			"-f", "s16le",
			"-ar", "24000",
			"-ac", "1",
			"-v", "quiet",
			"-acodec", "pcm_s16le",
			tmp2
		});

		Utils::exec({
			"./silk2mp3/encoder", 
			tmp2,
			audio_ans_path,
			"-rate", "24000",
			"-tencent", "-quiet"
		});
	}
	logging::INFO("音频处理完毕 <pjskSongGuess>: " + audio_ans_path);
	
	for (int i = 0 ; i < round; ++i)
	{
		string audio_path;
		{
			uuids::basic_uuid_random_generator rng(Utils::rng_engine);
			string tmp1 = MediaFilesPath + "tmp/" + to_string(rng()) + ".mp3";
			string tmp2 = MediaFilesPath + "tmp/" + to_string(rng()) + ".pcm";
			audio_path = MediaFilesPath + "tmp/" + to_string(rng()) + ".slk";

			
			double inter_2 = interval * (i * incr + 1.0) / 2.0;
			string apad = "";
			if (2.0 * inter_2 < 0.9)
				apad = ",apad=pad_dur=" + to_string(int(ceil(900 - 2000 * inter_2))) + "ms";

			Utils::exec({"ffmpeg",
				     "-ss", to_string(pos - inter_2),
				     "-t", to_string(inter_2 * 2),
				     "-v", "quiet",
				     "-i", music_path,
				     "-acodec", "copy",
				     tmp1});

			Utils::exec({"ffmpeg",
				     "-i", tmp1,
				     "-f", "s16le",
				     "-af", "adelay=100ms:all=true" + apad, // Add padding
				     "-ar", "24000",
				     "-ac", "1",
				     "-v", "quiet",
				     "-acodec", "pcm_s16le",
				     tmp2});


			Utils::exec({"./silk2mp3/encoder",
				     tmp2,
				     audio_path,
				     "-rate", "24000",
				     "-tencent", "-quiet"});
		}
		logging::INFO("音频处理完毕 <pjskSongGuess>: " + audio_path);
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Add<Cyan::VoiceMessage>(Cyan::MiraiVoice{.Path = audio_path}));

		const auto tp = chrono::system_clock::now();
		bool flag = false;
		while (true)
		{
			State::Activity::AnswerInfo info;
			if (!state->WaitForAnswerUntil(tp + chrono::seconds(21), info))
				break;
			Utils::ToLower(info.answer);
			Utils::ReplaceMark(info.answer);
			if (alias_map.contains(info.answer))
			{
				logging::INFO("回答正确 <pjskSongGuess>: " + info.answer + "\t-> [" + gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")]");
				client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain("回答正确"), info.message_id);
				flag = true;
				break;
			}
		}
		if (flag) break;
	}

	string cover = MediaFilesPath + "images/pjsk/cover_small/" + music["assetbundleName"].get<string>();
	if (music["hasOriginCover"].get<bool>() && music["vocal"]["musicVocalType"].get<string>() == "original_song")
		cover += "_org";
	cover += "_small.png";
	string answer = music["title"].get<string>() + " - " + music["vocal"]["caption"].get<string>();
	if (music["vocal"]["characters"].size())
	{
		answer += "\nVo. ";
		for (const auto &p : music["vocal"]["characters"].items())
		{
			answer += p.value().get<string>() + "   ";
		}
	}

	logging::INFO("公布答案 <pjskSongGuess>: " + music["title"].get<string>()
					+ "\t-> [" + gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")]");
	client.Send(gm.Sender.Group.GID, Cyan::MessageChain()
				.Plain("正确答案是: \n" + answer + "\n")
				.Image({.Path = cover}));
	client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Add<Cyan::VoiceMessage>(Cyan::MiraiVoice{.Path = audio_ans_path}));

	return true;
}

}