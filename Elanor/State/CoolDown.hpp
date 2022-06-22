#ifndef _COOLDOWN_HPP_
#define _COOLDOWN_HPP_

#include <State/StateBase.hpp>
#include <third-party/date.h>
#include <mutex>
#include <string>
#include <sstream>
#include <unordered_map>
#include <optional>
#include <chrono>

class CoolDown : public StateBase
{
protected:
	mutable std::mutex mtx;
	
	struct command_state
	{
		bool isUsing = false;
		std::optional<std::chrono::system_clock::time_point> LastUsed = std::nullopt;
	};

	std::unordered_map<std::string, command_state> cd;

public:

	struct Holder
	{
	private:
		CoolDown* obj;
		std::string id;
	public:
		friend class CoolDown;
		Holder(CoolDown* cd, const std::string& str) : obj(cd), id(str) {}
		Holder(Holder&) = delete;
		~Holder()
		{
			if (!this->obj) return;
			std::lock_guard<std::mutex> lk(obj->mtx);
			obj->cd[id].isUsing = false;
			obj->cd[id].LastUsed = std::chrono::system_clock::now();
		}
	};

	std::unique_ptr<Holder> GetRemaining(const std::string& id, const std::chrono::seconds& cd, std::chrono::seconds& remaining)
	{
		using namespace date;
		using namespace std::chrono;
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->cd[id].isUsing)
		{
			remaining = cd;
			return nullptr;
		}

		if (!this->cd[id].LastUsed.has_value())
		{
			this->cd[id].isUsing = true;
			remaining = 0s;
			return std::make_unique<Holder>(this, id);
		}
		
		auto passed = floor<seconds>(system_clock::now() - this->cd[id].LastUsed.value());
		if (passed >= cd)
		{
			this->cd[id].isUsing = true;
			remaining = 0s;
			return std::make_unique<Holder>(this, id);
		}
		else
		{
			remaining = cd - passed;
			return nullptr;
		}
	}

	nlohmann::json Serialize() override 
	{
		using namespace std::chrono;
		nlohmann::json content;
		for (const auto& p : this->cd)
		{
			if (p.second.LastUsed.has_value())
			{
				auto t = time_point_cast<seconds>(*p.second.LastUsed).time_since_epoch();
				content[p.first] = t.count();
			}
		}
		return content; 
	}

	void Deserialize(const nlohmann::json& content) override 
	{
		for (const auto& p : content.items())
		{
			using namespace std::chrono;
			command_state state;
			state.isUsing = false;
			state.LastUsed = time_point<system_clock>(seconds(p.value()));
			this->cd[p.key()] = state;
		}
	}
};

#endif