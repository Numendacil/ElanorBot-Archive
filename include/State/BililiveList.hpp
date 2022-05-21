#ifndef _BILILIVE_LIST_HPP_
#define _BILILIVE_LIST_HPP_

#include "StateBase.hpp"
#include <map>
#include <vector>
#include <mutex>

class BililiveList : public StateBase
{
public:
	struct info
	{
		long room_id;
		bool broadcasted = false;
	};
protected:
	std::map<long, info> user_list;		// Map uid -> room_id
	mutable std::mutex mtx;
public:
	void Insert(long uid, long room_id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.emplace(uid, info{room_id, false});
	}

	void Erase(long uid)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.erase(uid);
	}

	void Broadcast(long uid)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.at(uid).broadcasted = true;
	}

	void ResetBroadcast(long uid)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->user_list.at(uid).broadcasted = false;
	}

	bool Exist(long uid) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->user_list.count(uid);
	}

	std::vector<std::pair<long, info>> GetList(void) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return std::vector<std::pair<long, info>>(this->user_list.begin(), this->user_list.end());
	}

	virtual nlohmann::json Serialize() override
	{ 
		nlohmann::json content;
		for (const auto& u : this->user_list)
		{
			nlohmann::json member;
			member["uid"] = u.first;
			member["room_id"] = u.second.room_id;
			member["broadcasted"] = u.second.broadcasted;
			content += member;
		}
		return content;
	}

	virtual void Deserialize(const nlohmann::json& content) override
	{
		for (const auto& p : content.items())
		{
			nlohmann::json member = p.value();
			this->user_list.emplace(member["uid"].get<long>(), info{member["room_id"].get<long>(), member["broadcasted"].get<bool>()});
		}
	}
};

#endif