#include <filesystem>
#include <fstream>
#include "Timer.hpp"
#include "ElanorBot.hpp"
#include "Factory.hpp"
#include "State/State.hpp"
#include "Command/GroupCommandBase.hpp"
#include "utils/json.hpp"
#include "utils/log.h"

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

ElanorBot::ElanorBot(GID_t group_id, QQ_t owner_id) : gid(group_id), suid(owner_id)
{
	vector<string> list = Factory<GroupCommandBase>::GetKeyList();
	for (const auto& s : list)
	{
		auto ptr = Factory<GroupCommandBase>::Make(s);
		this->CommandAuth[s] = ptr->AuthRequirement();
	}

	list = Factory<StateBase>::GetKeyList();
	for (const auto& s : list)
		this->States[s] = Factory<StateBase>::Make(s);

	this->FromFile();
	Timer::GetInstance().LaunchLoop([this]{ this->ToFile(); }, chrono::minutes(60), true);
}

void ElanorBot::ToFile(void)
{
	json content;
	{
		lock_guard<mutex> lk(this->mtx);
		for (const auto& p : this->WhiteList)
			content["WhiteList"] += p.ToInt64();
		for (const auto& p : this->CommandAuth)
			content["CommandAuth"][p.first] = p.second;
		for (const auto& p : this->States)
			if (!p.second->Serialize().empty())
				content["States"][p.first] = p.second->Serialize();
	}
	filesystem::path Path = "./bot";
	Path /= to_string(this->gid.ToInt64());
	{
		lock_guard<mutex> lk(this->mtx_file);
		ofstream file(Path);
		if (!file)
		{
			logging::WARN("Failed to open file " + string(Path) + " for writing");
			return;
		}

		file << content.dump();
	}
}

void ElanorBot::FromFile(void)
{
	json content;
	filesystem::path Path = "./bot";
	Path /= to_string(this->gid.ToInt64());
	if (!filesystem::exists(Path))
		return;
	try
	{
		lock_guard<mutex> lk(this->mtx_file);
		ifstream file(Path);
		if (!file)
		{
			logging::WARN("Failed to open file " + string(Path) + " for reading");
			return;
		}
		content = json::parse(file);
	}
	catch(json::parse_error& e)
	{
		logging::WARN("Failed to parse file " + string(Path) + " :" + e.what());
		return;
	}

	{
		lock_guard<mutex> lk(this->mtx);
		if (content.contains("WhiteList"))
		{
			assert(content["WhiteList"].type() == json::value_t::array);
			for (const auto& p : content["WhiteList"].items())
				this->WhiteList.insert((QQ_t)p.value());
		}
		if (content.contains("CommandAuth"))
		{
			assert(content["CommandAuth"].type() == json::value_t::object);
			for (const auto& p : content["CommandAuth"].items())
				this->CommandAuth.emplace(p.key(), p.value());
		}
		if (content.contains("States"))
		{
			assert(content["States"].type() == json::value_t::object);
			for (const auto& p : content["States"].items())
				if (this->States.count(p.key()))
					this->States[p.key()]->Deserialize(p.value());
		}
	}
}