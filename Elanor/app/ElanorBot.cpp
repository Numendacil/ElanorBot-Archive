#include <filesystem>
#include <fstream>

#include <Utils/Timer.hpp>
#include <Utils/Factory.hpp>

#include <app/ElanorBot.hpp>

#include <State/State.hpp>

#include <Command/GroupCommandBase.hpp>

#include <third-party/json.hpp>
#include <third-party/log.h>

using namespace std;
using namespace Cyan;
using json = nlohmann::json;

ElanorBot::ElanorBot(GID_t group_id, QQ_t owner_id, shared_ptr<MiraiBot> client) : gid(group_id), suid(owner_id)
{
	this->client = client;
	vector<string> list = Factory<GroupCommandBase>::GetKeyList();
	for (const auto& s : list)
	{
		auto ptr = Factory<GroupCommandBase>::Make(s);
		this->CommandAuth[s] = ptr->AuthRequirement();
	}

	list = Factory<StateBase>::GetKeyList();
	for (const auto& s : list)
		this->States[s] = Factory<StateBase>::Make(s);

	list = Factory<TriggerBase>::GetKeyList();
	for (const auto& s : list)
	{
		this->Triggers[s] = Factory<TriggerBase>::Make(s);
		this->Trigger_Enabled[s] = this->Triggers[s]->TriggerOnStart();
	}

	this->FromFile();

	for (const auto& s : this->Triggers)
	{
		if (this->Trigger_Enabled[s.first])
			s.second->trigger_on(client, this);
	}

	Timer::GetInstance().LaunchLoop([this]{ this->ToFile(); }, chrono::hours(1), true);
}

void ElanorBot::ToFile(void)
{
	json content;
	{
		lock_guard<mutex> lk(this->mtx);
		for (const auto& p : this->WhiteList)
			content["WhiteList"] += p.ToInt64();
		for (const auto& p : this->BlackList)
			content["BlackList"] += p.ToInt64();
		for (const auto& p : this->CommandAuth)
			content["CommandAuth"][p.first] = p.second;
		for (const auto& p : this->States)
			if (!p.second->Serialize().empty())
				content["States"][p.first] = p.second->Serialize();
		for (const auto& p : this->Trigger_Enabled)
			content["Triggers"][p.first] = p.second;
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
		logging::INFO("Writing to file " + string(Path));
		file << content.dump(1, '\t');
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
	logging::INFO("Reading from file " + string(Path));
	{
		lock_guard<mutex> lk(this->mtx);
		if (content.contains("WhiteList"))
		{
			assert(content["WhiteList"].type() == json::value_t::array);
			for (const auto& p : content["WhiteList"].items())
				this->WhiteList.insert((QQ_t)p.value());
		}
		if (content.contains("BlackList"))
		{
			assert(content["BlackList"].type() == json::value_t::array);
			for (const auto& p : content["BlackList"].items())
				this->BlackList.insert((QQ_t)p.value());
		}
		if (content.contains("CommandAuth"))
		{
			assert(content["CommandAuth"].type() == json::value_t::object);
			for (const auto& p : content["CommandAuth"].items())
				if (this->CommandAuth.count(p.key()))
					this->CommandAuth[p.key()] = p.value();
		}
		if (content.contains("States"))
		{
			assert(content["States"].type() == json::value_t::object);
			for (const auto& p : content["States"].items())
				if (this->States.count(p.key()))
					this->States[p.key()]->Deserialize(p.value());
		}
		if (content.contains("Triggers"))
		{
			assert(content["Triggers"].type() == json::value_t::object);
			for (const auto& p : content["Triggers"].items())
				if (this->Trigger_Enabled.count(p.key()))
					this->Trigger_Enabled[p.key()] = p.value();
		}
	}
}