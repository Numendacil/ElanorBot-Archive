#include "Command/pjsk/pjskSongGuess.hpp"
#include "State/Activity.hpp"
#include "third-party/log.h"
#include "third-party/uuid.h"
#include "third-party/json.hpp"
#include "ElanorBot.hpp"
#include "Utils.hpp"
#include "MessageQueue.hpp"
#include <mirai/messages/messages.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

using namespace std;
using namespace Cyan;
using namespace httplib_ssl_zlib;
using json = nlohmann::json;


bool pjskSongGuess::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk 猜歌"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(token, str) < 2)
			return false;
		if (token[0] == "#pjsk" || token[0] == "#啤酒烧烤" || token[0] == "#prsk")
			if (token[1] == "猜歌")
				return true;
	}
	return false;
}



bool pjskSongGuess::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling pjskSongGuess <pjskSongGuess>" + Utils::GetDescription(gm));

	auto state = bot->GetState<Activity>("Activity");
	auto holder = state->CheckAndStart("pjsk");
	const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
	if (!holder)
	{
		logging::INFO("有活动正在进行 <pjskSongGuess>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("有活动正在进行中捏"));
		return false;
	}

	Utils::SendGroupMessage(gm, MessageChain().Plain("请在规定时间内发送音频选段对应的歌曲名称。回答请以句号开头捏"));

	json music, alias;
	{
		ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
		json meta_data = json::parse(ifile);
		ifile.close();

		uniform_int_distribution<int> rng1(0, meta_data.size() - 1);
		music = meta_data[rng1(Utils::rng_engine)];
		uniform_int_distribution<int> rng2(0, music["vocal"].size() - 1);
		music["vocal"] = music["vocal"][rng2(Utils::rng_engine)];
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
	double interval = Utils::Configs.Get<double>("/pjsk/SongGuess/Interval"_json_pointer, 1);
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
		double max_inter_2 = interval * ((round - 1) * 0.8f + 1.0f) / 2.0f;
		double ans_interval_2 = 4;
		uniform_real_distribution<double> rng_real(2.0 + ans_interval_2 + max_inter_2, length - max_inter_2 - 2.0 - ans_interval_2);
		pos = rng_real(Utils::rng_engine);

		Utils::exec({
			"ffmpeg",
			"-ss", to_string(pos - ans_interval_2 - max_inter_2),
			"-t", to_string((max_inter_2 + ans_interval_2) * 2.0f),
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

			
			double inter_2 = interval * (i * 0.8f + 1.0f) / 2.0f;
			string apad = "";
			if (2.0f * inter_2 < 0.9f)
				apad = ",apad=pad_dur=" + to_string(int(ceil(900 - 200 * inter_2))) + "ms";

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
		Utils::SendGroupMessage(gm, MessageChain().Add<VoiceMessage>(MiraiVoice{"", "", audio_path, ""}));

		const auto tp = chrono::system_clock::now();
		bool flag = false;
		while (true)
		{
			Activity::AnswerInfo info;
			if (!state->WaitForAnswerUntil(tp + chrono::seconds(21), info))
				break;
			Utils::ToLower(info.answer);
			Utils::ReplaceMark(info.answer);
			if (alias_map.contains(info.answer))
			{
				logging::INFO("回答正确 <pjskSongGuess>: " + info.answer + "\t-> [" + gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")]");
				MessageQueue::GetInstance().Push(gm.Sender.Group.GID, MessageChain().Plain("回答正确"), info.message_id);
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
	Utils::SendGroupMessage(gm, MessageChain()
				.Plain("正确答案是: \n" + answer + "\n")
				.Image({"", "", cover, ""}));
	Utils::SendGroupMessage(gm, MessageChain().Add<VoiceMessage>(MiraiVoice{"", "", audio_ans_path, ""}));

	return true;
}