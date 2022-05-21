#include <filesystem>
#include "third-party/log.h"
#include "Command/AtBot.hpp"
#include "Utils.hpp"
#include "ElanorBot.hpp"

using namespace std;
using namespace Cyan;

bool AtBot::Parse(const MessageChain& msg, vector<string>& token)
{
	if (msg.GetAll<AtMessage>().empty())
		return false;
	return true;
}

bool AtBot::Execute(const GroupMessage& gm, shared_ptr<ElanorBot> bot, const vector<string>& token)
{
	if (!gm.AtMe())
		return false;

	logging::INFO("有人@bot <AtBot>" + Utils::GetDescription(gm));
	const vector<string> words = {"干嘛", "？"};
	const int words_count = words.size();

	filesystem::path filepath = Utils::MediaFilePath + "images/at";
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
		Utils::SendGroupMessage(gm, MessageChain().Image({"", "", image[idx - words_count], ""}));
	}
	else
	{
		logging::INFO("回复 <AtBot>: " + words[idx] + Utils::GetDescription(gm, false));
		Utils::SendGroupMessage(gm, MessageChain().Plain(words[idx]));
	}
	return true;
}