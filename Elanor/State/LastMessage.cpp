#include "LastMessage.hpp"
#include "mirai/third-party/nlohmann/json.hpp"

using json = nlohmann::json;

namespace State
{

void LastMessage::Set(const Cyan::MessageChain &m, bool r)
{
	std::lock_guard<std::mutex> lk(this->mtx);
	this->LastMsg = m;
	this->Repeated = r;
}

void LastMessage::Get(Cyan::MessageChain &m, bool &r) const
{
	std::lock_guard<std::mutex> lk(this->mtx);
	m = this->LastMsg;
	r = this->Repeated;
}

json LastMessage::Serialize()
{
	std::lock_guard<std::mutex> lk(this->mtx);
	json content;
	content["Message"] = this->LastMsg.ToJson();
	content["Repeated"] = this->Repeated;
	return content; 
}

void LastMessage::Deserialize(const json &content)
{ 
	std::lock_guard<std::mutex> lk(this->mtx);
	if (content.contains("Message"))
	{
		this->LastMsg = Cyan::MessageChain();
		this->LastMsg.Set(content["Message"]);
	}
	if (content.contains("Repeated"))
		this->Repeated = content["Repeated"].get<bool>();
}

}