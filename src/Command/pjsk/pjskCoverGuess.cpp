#include "Command/pjsk/pjskCoverGuess.hpp"
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


bool pjskCoverGuess::Parse(const MessageChain& msg, vector<string>& token)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#pjsk 猜曲绘"))
	{
		Utils::ToLower(str);
		if (Utils::Tokenize(token, str) < 2)
			return false;
		if (token[0] == "#pjsk" || token[0] == "#啤酒烧烤" || token[0] == "#prsk")
			if (token[1] == "猜曲绘")
				return true;
	}
	return false;
}



bool pjskCoverGuess::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	assert(token.size() > 1);
	logging::INFO("Calling pjskCoverGuess <pjskCoverGuess>" + Utils::GetDescription(gm));

	auto state = bot->GetState<Activity>("Activity");
	auto holder = state->CheckAndStart("pjsk");
	const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
	if (!holder)
	{
		logging::INFO("有活动正在进行 <pjskCoverGuess>" + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain("有活动正在进行中捏"));
		return false;
	}

	Utils::SendGroupMessage(gm, MessageChain().Plain("请在30秒内发送曲绘对应的歌曲名称。回答请以句号开头捏"));

	json music, alias;
	{
		ifstream ifile(MediaFilesPath + "music/pjsk/meta.json");
		json meta_data = json::parse(ifile);
		ifile.close();

		uniform_int_distribution<int> rng1(0, meta_data.size() - 1);
		music = meta_data[rng1(Utils::rng_engine)];
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
	logging::INFO("题目为 <pjskCoverGuess>: " + music.dump() + alias.dump());


	string cover_path, cover_ans_path;
	{
		uuids::basic_uuid_random_generator rng(Utils::rng_engine);
		string cover = music["assetbundleName"].get<string>();
		if (music["hasOriginCover"].get<bool>())
		{
			bernoulli_distribution rng2(0.5);
			cover += (rng2(Utils::rng_engine))? "" : "_org";
		}
		cover_ans_path = MediaFilesPath + "images/pjsk/cover_small/" + cover + "_small.png";
		cover = MediaFilesPath + "images/pjsk/cover/" + cover + ".png";
		cover_path = MediaFilesPath + "tmp/" + to_string(rng()) + ".png";
		int width, height;
		const string shape = Utils::exec({
			"identify",
			"-format", "%[fx:w] %[fx:h]",
			cover
		});
		vector<string> s;
		if (Utils::Tokenize(s, shape) < 2)
		{
			logging::WARN("Failed to get image size <pjskCoverGuess>: " + shape);
			width = 780;
			height = 780;
		}
		else
		{
			width = stoi(s[0]);
			height = stoi(s[1]);
		}

		const double ratio = Utils::Configs.Get<double>("/pjsk/CoverGuess/Ratio"_json_pointer, 0.15);
		int width_target = (int)floor(width * ratio);
		int height_target = (int)floor(height * ratio);
		uniform_int_distribution<int> rng_x(5, width - 5 - width_target);
		uniform_int_distribution<int> rng_y(5, height - 5 - height_target);
		int x = rng_x(Utils::rng_engine);
		int y = rng_y(Utils::rng_engine);

		Utils::exec({
			"convert",
			"-crop", to_string(width_target) + "x"+ to_string(height_target) + "+" + to_string(x) + "+" + to_string(y),
			cover,
			cover_path
		});
	}
	logging::INFO("图片处理完毕 <pjskCoverGuess>: " + cover_path);
	Utils::SendGroupMessage(gm, MessageChain().Image({"", "", cover_path, ""}));

	string answer = music["title"].get<string>();
	
	{
		unordered_set<string> alias_map;
		for (const auto& item : alias.items())
		{
			string str = string(item.value());
			Utils::ToLower(str);
			Utils::ReplaceMark(str);
			alias_map.insert(str);
		}
		const auto tp = chrono::system_clock::now();
		while (true)
		{
			Activity::AnswerInfo info;
			if (!state->WaitForAnswerUntil(tp + chrono::seconds(32), info))
				break;
			Utils::ToLower(info.answer);
			Utils::ReplaceMark(info.answer);
			if (alias_map.contains(info.answer))
			{
				logging::INFO("回答正确 <pjskCoverGuess>: " + info.answer 
					+ "\t-> [" + gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")]");
				MessageQueue::GetInstance().Push(gm.Sender.Group.GID, MessageChain().Plain("回答正确"), info.message_id);
				break;
			}
		}
	}

	logging::INFO("公布答案 <pjskCoverGuess>: " + music["title"].get<string>()
					+ "\t-> [" + gm.Sender.Group.Name + "(" + to_string(gm.Sender.Group.GID.ToInt64()) + ")]");
	Utils::SendGroupMessage(gm, MessageChain()
				.Plain("正确答案是:   " + answer + "\n")
				.Image({"", "", cover_ans_path, ""}));

	return true;
}