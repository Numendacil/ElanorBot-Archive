#ifndef _GROUP_HPP_
#define _GROUP_HPP_

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <mutex>
#include <mirai/defs/defs.hpp>
#include <State/StateBase.hpp>

#include <ThirdParty/log.h>
#include <Utils/Factory.hpp>
#include <utility>
#include <vector>

namespace Bot
{

class Group
{

protected:
	mutable std::mutex mtx;
	mutable std::mutex mtx_file;

	const std::unordered_map<std::string, std::unique_ptr<State::StateBase>> States;
public:
	Group(Cyan::GID_t group_id, Cyan::QQ_t owner_id, 
		const std::vector<std::pair<std::string, int>>& command_list,
		const std::vector<std::pair<std::string, bool>>& trigger_list);
	void ToFile();
	void FromFile();

	const Cyan::GID_t gid;
	const Cyan::QQ_t suid;

	template<class T>
	T* GetState(const std::string& str) const
	{
		static_assert(std::is_base_of<State::StateBase, T>::value, "T must be a derived class of StateBase.");
		std::lock_guard<std::mutex> lk(this->mtx);
		assert(this->States.count(str));
		auto ptr = this->States.at(str).get();
		assert(ptr != nullptr);
		return static_cast<T*>(ptr);
	}

	~Group() 
	{
		this->ToFile(); 
	}
};

}

#endif