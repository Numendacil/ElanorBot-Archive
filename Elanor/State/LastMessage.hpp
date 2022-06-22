#ifndef _LAST_REPEAT_HPP_
#define _LAST_REPEAT_HPP_

#include <State/StateBase.hpp>
#include <mutex>
#include <mirai/defs/MessageChain.hpp>

class LastMessage : public StateBase
{
protected:
	Cyan::MessageChain LastMsg = Cyan::MessageChain();
	bool Repeated = false;
	mutable std::mutex mtx;

public:
	void Set(const Cyan::MessageChain& m, bool r)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		this->LastMsg = m;
		this->Repeated = r;
	}

	void Get(Cyan::MessageChain& m, bool& r) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		m = this->LastMsg;
		r = this->Repeated;
	}

	nlohmann::json Serialize() override 
	{
		nlohmann::json content;
		content["Message"] = this->LastMsg.ToJson();
		content["Repeated"] = this->Repeated;
		return content; 
	}

	void Deserialize(const nlohmann::json& content) override 
	{ 
		if (content.contains("Message"))
		{
			this->LastMsg = Cyan::MessageChain();
			this->LastMsg.Set(content["Message"]);
		}
		if (content.contains("Repeated"))
			this->Repeated = content["Repeated"].get<bool>();
	}
};

#endif