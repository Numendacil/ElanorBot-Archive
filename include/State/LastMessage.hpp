#ifndef _LAST_REPEAT_HPP_
#define _LAST_REPEAT_HPP_

#include "StateBase.hpp"
#include <mirai/defs/MessageChain.hpp>

class LastMessage : public StateBase
{
public:
	Cyan::MessageChain LastMsg = Cyan::MessageChain();
	bool Repeated = false;

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