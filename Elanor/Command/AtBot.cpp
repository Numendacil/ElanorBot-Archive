#include <filesystem>
#include <ThirdParty/log.h>
#include <Utils/Utils.hpp>
#include <Group/Group.hpp>
#include <Client/Client.hpp>

#include "AtBot.hpp"

using namespace std;

namespace GroupCommand
{

bool AtBot::Parse(const Cyan::MessageChain& msg, vector<string>& tokens)
{
	if (msg.GetAll<Cyan::AtMessage>().empty())
		return false;
	return true;
}

bool AtBot::Execute(const Cyan::GroupMessage& gm, Group& group, const vector<string>& tokens) 
{
	if (!gm.AtMe())
		return false;

	Client& client = Client::GetClient();

	logging::INFO("有人@bot <AtBot>" + Utils::GetDescription(gm));
	const vector<string> words = {"干嘛", "？"};
	const int words_count = words.size();

	const filesystem::path filepath = Utils::Configs.Get<string>("/MediaFiles"_json_pointer, "media_files/") + "images/at";
	vector<string> image;
	for (const auto & entry : filesystem::directory_iterator(filepath))
	{
		if (filesystem::is_regular_file(entry))
			image.push_back(entry.path());
	}

	int idx = uniform_int_distribution<int>(0, words_count + image.size() - 1)(Utils::rng_engine);
	if (idx >= words_count)
	{
		logging::INFO("回复 <AtBot>: " + image[idx - words_count] + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Image({.Path = image[idx - words_count]}));
	}
	else
	{
		logging::INFO("回复 <AtBot>: " + words[idx] + Utils::GetDescription(gm, false));
		client.Send(gm.Sender.Group.GID, Cyan::MessageChain().Plain(words[idx]));
	}
	return true;
}

}