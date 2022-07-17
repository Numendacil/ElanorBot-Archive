#include "TriggerStatus.hpp"
#include <ThirdParty/json.hpp>

using json = nlohmann::json;

namespace State
{

std::vector<std::string> TriggerStatus::GetTriggerList() const
{
	std::vector<std::string> v;
	v.reserve(this->Enabled.size());
	for (const auto& p : this->Enabled)
		v.push_back(p.first);
	return v;
}

json TriggerStatus::Serialize()
{
	using namespace std::chrono;
	json content;
	for (const auto& p : this->Enabled)
	{
		content[p.first] = p.second;
	}
	return content; 
}

void TriggerStatus::Deserialize(const json &content)
{
	for (const auto& p : content.items())
	{
			this->Enabled[p.key()] = p.value().get<bool>();

	}
}

}