#ifndef _ACCESS_CONTROL_LIST_HPP_
#define _ACCESS_CONTROL_LIST_HPP_

#include <mirai/defs/QQType.hpp>
#include <State/StateBase.hpp>
#include <mutex>
#include <string>
#include <unordered_set>

namespace State
{

class AccessCtrlList : public StateBase
{
protected:
	mutable std::mutex mtx;
	
	std::unordered_set<Cyan::QQ_t> WhiteList;
	std::unordered_set<Cyan::QQ_t> BlackList;

public:

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

	virtual nlohmann::json Serialize() override;
	virtual void Deserialize(const nlohmann::json& content) override;

};

}

#endif