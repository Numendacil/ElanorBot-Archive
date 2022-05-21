#include "Command/pjsk/pjskSongGuess.hpp"
#include "State/Activity.hpp"
#include "utils/log.h"
#include "utils/uuid.h"
#include "utils/json.hpp"
#include "ElanorBot.hpp"
#include "Common.hpp"
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
	Common::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk guess"))
	{
		Common::ToLower(str);
		if (Common::Tokenize(token, str) < 2)
			return false;
		if (token[0] == "#pjsk" || token[0] == "#啤酒烧烤" || token[0] == "#prsk")
			if (token[1] == "guess" || token[1] == "猜歌")
			return true;
	}
	return false;
}



bool pjskSongGuess::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling pjskSongGuess <pjskSongGuess>" + Common::GetDescription(gm));

	auto state = bot->GetState<Activity>("Activity");
	auto holder = state->CheckAndStart("pjsk");
	if (!holder)
	{
		logging::INFO("有活动正在进行 <pjskSongGuess>" + Common::GetDescription(gm, false));
		Common::SendGroupMessage(gm, MessageChain().Plain("有活动正在进行中捏"));
		return false;
	}

	Common::SendGroupMessage(gm, MessageChain().Plain("请在30秒内发送音频选段对应的歌曲名称。回答请以句号开头捏"));

	json music, alias;
	{
		ifstream ifile(Common::MediaFilePath + "music/pjsk/meta.json");
		json meta_data = json::parse(ifile);
		ifile.close();

		uniform_int_distribution<int> rng1(0, meta_data.size() - 1);
		music = meta_data[rng1(Common::rng_engine)];
		uniform_int_distribution<int> rng2(0, music["vocal"].size() - 1);
		music["vocal"] = music["vocal"][rng2(Common::rng_engine)];
	}
	{
		ifstream ifile(Common::MediaFilePath + "music/pjsk/alias.json");
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


	string audio_path, audio_ans_path;
	{
		uuids::basic_uuid_random_generator rng(Common::rng_engine);
		string music_path = Common::MediaFilePath + "music/pjsk/songs/" + music["vocal"]["assetbundleName"].get<string>() + ".mp3";
		string tmp1 = Common::MediaFilePath + "tmp/" + to_string(rng()) + ".mp3";
		string tmp2 = Common::MediaFilePath + "tmp/" + to_string(rng()) + ".pcm";
		audio_path = Common::MediaFilePath + "tmp/" + to_string(rng()) + ".slk";

		string tmp3 = Common::MediaFilePath + "tmp/" + to_string(rng()) + ".mp3";
		string tmp4 = Common::MediaFilePath + "tmp/" + to_string(rng()) + ".pcm";
		audio_ans_path = Common::MediaFilePath + "tmp/" + to_string(rng()) + ".slk";

		double length = music["length"].get<double>();
		const double interval = 1.0;
		const double ans_interval_2 = 4;
		uniform_real_distribution<double> rng_real(2.0 + ans_interval_2, length - interval - 2.0 - ans_interval_2);
		double start = rng_real(Common::rng_engine);

		Common::exec({
			"ffmpeg",
			"-ss", to_string(start),
			"-t", to_string(interval),
			"-v", "quiet",
			"-i", music_path, 
			"-acodec", "copy", 
			tmp1
		});

		Common::exec({
			"ffmpeg",
			"-ss", to_string(start - ans_interval_2),
			"-t", to_string(interval + 2 * ans_interval_2),
			"-v", "quiet",
			"-i", music_path, 
			"-acodec", "copy", 
			tmp3
		});

		Common::exec({
			"ffmpeg", 
			"-i", tmp1, 
			"-f", "s16le",
			"-ar", "24000",
			"-ac", "1",
			"-v", "quiet",
			"-acodec", "pcm_s16le",
			tmp2
		});

		Common::exec({
			"ffmpeg", 
			"-i", tmp3, 
			"-f", "s16le",
			"-ar", "24000",
			"-ac", "1",
			"-v", "quiet",
			"-acodec", "pcm_s16le",
			tmp4
		});

		Common::exec({
			"./silk2mp3/encoder", 
			tmp2,
			audio_path,
			"-rate", "24000",
			"-tencent", "-quiet"
		});

		Common::exec({
			"./silk2mp3/encoder", 
			tmp4,
			audio_ans_path,
			"-rate", "24000",
			"-tencent", "-quiet"
		});
	}
	logging::INFO("音频处理完毕 <pjskSongGuess>: " + audio_path);
	Common::SendGroupMessage(gm, MessageChain().Add<VoiceMessage>(MiraiVoice{"", "", audio_path, ""}));

	string cover = Common::MediaFilePath + "music/pjsk/cover/" + music["assetbundleName"].get<string>();
	if (music["hasOriginCover"].get<bool>() && music["vocal"]["musicVocalType"].get<string>() == "original_song")
		cover += "_org";
	cover += ".png";
	string answer = music["title"].get<string>() + " - " + music["vocal"]["caption"].get<string>();
	if (music["vocal"]["characters"].size())
	{
		answer += "\nVo. ";
		for (const auto &p : music["vocal"]["characters"].items())
		{
			answer += p.value().get<string>() + "   ";
		}
	}
	
	{
		unordered_set<string> alias_map;
		for (const auto& item : alias.items())
		{
			string str = string(item.value());
			Common::ToLower(str);
			Common::ReplaceMark(str);
			alias_map.insert(str);
		}
		const auto tp = chrono::system_clock::now();
		while (true)
		{
			Activity::AnswerInfo info;
			if (!state->WaitForAnswerUntil(tp + chrono::seconds(32), info))
				break;
			Common::ToLower(info.answer);
			Common::ReplaceMark(info.answer);
			if (alias_map.contains(info.answer))
			{
				logging::INFO("回答正确 <pjskSongGuess>: " + info.answer 
					+ "\t-> [" + gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")]");
				MessageQueue::GetInstance().Push(gm.Sender.Group.GID, MessageChain().Plain("回答正确"), info.message_id);
				break;
			}
		}
	}

	logging::INFO("公布答案 <pjskSongGuess>: " + music["title"].get<string>()
					+ "\t-> [" + gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")]");
	Common::SendGroupMessage(gm, MessageChain()
				.Plain("正确答案是: \n" + answer + "\n")
				.Image({"", "", cover, ""}));
	Common::SendGroupMessage(gm, MessageChain().Add<VoiceMessage>(MiraiVoice{"", "", audio_ans_path, ""}));

	return true;
}