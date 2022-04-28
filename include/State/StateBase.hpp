#ifndef _STATE_BASE_HPP_
#define _STATE_BASE_HPP_

#include <string>
#include <mutex>
#include "utils/json.hpp"

class StateBase
{
public:
	// Return empty string if no serialization needed
	virtual nlohmann::json Serialize() { return nlohmann::json(); }
	virtual void Deserialize(const nlohmann::json&) {}

	std::unique_lock<std::mutex> GetLock() const { return std::unique_lock<std::mutex>(this->mtx); }
	void ReleaseLock(std::unique_lock<std::mutex>&& l) const { std::unique_lock<std::mutex> Lock = std::move(l); }

private:
	mutable std::mutex mtx;
};

#endif