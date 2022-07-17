#include "CommandPerm.hpp"
#include <ThirdParty/json.hpp>

using json = nlohmann::json;

namespace State
{

std::vector<std::string> CommandPerm::GetCommandList() const
{
	std::vector<std::string> v;
	v.reserve(this->Permission.size());
	for (const auto& p : this->Permission)
		v.push_back(p.first);
	return v;
}

json CommandPerm::Serialize()
{
	using namespace std::chrono;
	json content;
	for (const auto& p : this->Permission)
	{
		content[p.first]["current"] = p.second.first;
		content[p.first]["default"] = p.second.second;
	}
	return content; 
}

void CommandPerm::Deserialize(const json &content)
{
	for (const auto& p : content.items())
	{
		if (p.value().contains("current"))
			this->Permission[p.key()].first = p.value()["current"].get<int>();
		if (p.value().contains("default"))
			this->Permission[p.key()].first = p.value()["default"].get<int>();

	}
}

}