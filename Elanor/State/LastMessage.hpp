#ifndef _LAST_REPEAT_HPP_
#define _LAST_REPEAT_HPP_

#include <State/StateBase.hpp>
#include <mutex>
#include <mirai/defs/MessageChain.hpp>

namespace State
{

class LastMessage : public StateBase
{
protected:
	Cyan::MessageChain LastMsg = Cyan::MessageChain();
	bool Repeated = false;
	mutable std::mutex mtx;

public:

	static constexpr std::string_view _NAME_ = "LastMessage";

	void Set(const Cyan::MessageChain& m, bool r);
	void Get(Cyan::MessageChain& m, bool& r) const;

	virtual nlohmann::json Serialize() override;
	virtual void Deserialize(const nlohmann::json& content) override;
};

}
#endif