#ifndef _ELANOR_BOT_HPP_
#define _ELANOR_BOT_HPP_

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <mutex>
#include <mirai/defs/defs.hpp>
#include <mirai/MiraiBot.hpp>
#include <State/StateBase.hpp>
#include <Trigger/TriggerBase.hpp>

#include <ThirdParty/log.h>

class ElanorBot
{

protected:
	mutable std::mutex mtx;
	mutable std::mutex mtx_file;

	std::unordered_set<Cyan::QQ_t> WhiteList;
	std::unordered_set<Cyan::QQ_t> BlackList;
	std::unordered_map<std::string, int> CommandAuth;
	std::unordered_map<std::string, bool> Trigger_Enabled;
	std::unordered_map<std::string, std::shared_ptr<StateBase>> States;
	std::unordered_map<std::string, std::shared_ptr<TriggerBase>> Triggers;
public:
	ElanorBot(Cyan::GID_t group_id, Cyan::QQ_t owner_id, std::shared_ptr<Cyan::MiraiBot> client);
	void ToFile();
	void FromFile();

	const Cyan::GID_t gid;
	const Cyan::QQ_t suid;
	std::shared_ptr<Cyan::MiraiBot> client;



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



	bool IsBlackList(const Cyan::QQ_t& id) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->BlackList.contains(id);
	}

	void BlackListAdd(const Cyan::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->BlackList.insert(id);
	}

	void BlackListDelete(const Cyan::QQ_t& id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->BlackList.erase(id);
	}

	void BlackListClear(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->BlackList.clear();
	}
	
	std::vector<Cyan::QQ_t> GetBlackList(void)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return std::vector<Cyan::QQ_t>(this->BlackList.begin(), this->BlackList.end());
	}



	bool ExistCommand(const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->CommandAuth.count(command);
	}

	int GetCommandAuth(const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->CommandAuth.count(command));
		return this->CommandAuth.at(command);
	}

	bool CheckAuth(const Cyan::GroupMember& member, const std::string& command) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->CommandAuth.count(command));
		int auth = 0;
		if (member.QQ == this->suid)
		//	auth = 100;
			return true;
		if (this->BlackList.contains(member.QQ))
			return false;

		switch (member.Permission)
		{
		case Cyan::GroupPermission::Member:
			auth = 0; break;
		case Cyan::GroupPermission::Administrator:
			auth = 10; break;
		case Cyan::GroupPermission::Owner:
			auth = 20; break;
		}

		if (this->WhiteList.contains(member.QQ))
			auth = 50;
		
		return auth >= this->CommandAuth.at(command);
	}

	void UpdateAuth(const std::string& command, int auth)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->CommandAuth.count(command));
		this->CommandAuth[command] = auth;
	}


	bool ExistTrigger(const std::string& trig) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->Triggers.count(trig);
	}

	bool GetTriggerStatus(const std::string& trig) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->Trigger_Enabled.count(trig));
		return this->Trigger_Enabled.at(trig);
	}
	
	void TriggerOn(const std::string& trig)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->Triggers.count(trig));
		this->Triggers.at(trig)->trigger_on(this->client, this);
		this->Trigger_Enabled.at(trig) = true;
	}

	void TriggerOff(const std::string& trig)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->Triggers.count(trig));
		this->Triggers.at(trig)->trigger_off();
		this->Trigger_Enabled.at(trig) = false;
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

	~ElanorBot() 
	{
		for (const auto& p : this->Triggers)
			if(p.second->IsRunning())
				p.second->trigger_off();
		this->ToFile(); 
	}
};

#endif