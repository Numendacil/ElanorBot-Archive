#include <ThirdParty/log.h>
#include <ThirdParty/uuid.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>
#include <mirai.h>

#include "Sing.hpp"

using namespace std;
using json = nlohmann::json;

namespace GroupCommand
{

bool Sing::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	string str = msg.GetPlainText();
	Utils::ReplaceMark(str);
	if (str.length() >= char_traits<char>::length("#sing"))
	{
		Utils::ToLower(str);
		Utils::Tokenize(tokens, str);
		if (tokens[0] == "#sing" || tokens[0] == "#唱歌")
			return true;
	}
	return false;
}

bool Sing::Execute(const Cyan::GroupMessage& gm, Bot::Group& group, const vector<string>& tokens) 
{
	Bot::Client& client = Bot::Client::GetClient();
	logging::INFO("Calling Sing <Sing>" + Utils::GetDescription(gm));
	
	const string MediaFilesPath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/");
	string music_path = MediaFilesPath + "music/a.mp3";
	uuids::basic_uuid_random_generator rng(Utils::rng_engine);
	string tmp = MediaFilesPath + "tmp/" + to_string(rng()) + ".pcm";
	string audio_path = MediaFilesPath + "tmp/" + to_string(rng()) + ".slk";

	Utils::exec({
		"ffmpeg", 
		"-i", music_path, 
		"-f", "s16le",
		"-ar", "24000",
		"-ac", "1",
		"-v", "quiet",
		"-acodec", "pcm_s16le",
		tmp
	});

	Utils::exec({
		"./silk2mp3/encoder", 
		tmp,
		audio_path,
		"-rate", "24000",
		"-tencent", "-quiet"
		});

	client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Add<Cyan::VoiceMessage>(Cyan::MiraiVoice{.Path = audio_path}));
	return true;
}

}