#ifndef _ELANOR_BOT_HPP_
#define _ELANOR_BOT_HPP_

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <mutex>
#include <mirai/defs/defs.hpp>
#include "State/StateBase.hpp"

#include "utils/log.h"

class ElanorBot
{
public:
	ElanorBot(Cyan::GID_t group_id, Cyan::QQ_t owner_id);
	~ElanorBot() { this->ToFile(); }
	void ToFile();
	void FromFile();



	bool IsWhiteList(const Cyan::QQ_t& id) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->WhiteList.contains(id);
	}

	void WhiteListAdd(const Cyan::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->WhiteList.insert(id);
	}

	void WhiteListDelete(const Cyan::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->WhiteList.erase(id);
	}

	void WhiteListClear(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->WhiteList.clear();
	}
	
	std::vector<Cyan::QQ_t> GetWhiteList(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return std::vector<Cyan::QQ_t>(this->WhiteList.begin(), this->WhiteList.end());
	}



	bool CheckAuth(const Cyan::GroupMember& member, const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->CommandAuth.count(command));
		int auth = 0;

		switch (member.Permission)
		{
		case Cyan::GroupPermission::Member:
			auth = 0; break;
		case Cyan::GroupPermission::Administrator:
			auth = 10; break;
		case Cyan::GroupPermission::Owner:
			auth = 20; break;
		}

		if (member.QQ == this->suid)
			auth = 100;
		else if (this->WhiteList.contains(member.QQ))
			auth = 50;
		
		return auth >= CommandAuth.at(command);
	}

	void UpdateAuth(const std::string& command, int auth)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->CommandAuth.count(command));
		this->CommandAuth[command] = auth;
	}



	template<class T>
	std::shared_ptr<T> GetState(const std::string& str) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->States.count(str));
		std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(this->States.at(str));
		assert(ptr != nullptr);
		return ptr;
	}

protected:
	const Cyan::GID_t gid;
	const Cyan::QQ_t suid;

	mutable std::mutex mtx;
	mutable std::mutex mtx_file;

	std::unordered_set<Cyan::QQ_t> WhiteList;
	std::unordered_map<std::string, int> CommandAuth;
	std::unordered_map<std::string, std::shared_ptr<StateBase>> States;
};

#endif