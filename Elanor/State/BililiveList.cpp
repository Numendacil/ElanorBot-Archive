#include "BililiveList.hpp"
#include <ThirdParty/json.hpp>

using json = nlohmann::json;

namespace State
{

json BililiveList::Serialize()
{
	nlohmann::json content;
	for (const auto& u : this->user_list)
	{
		nlohmann::json member;
		member["uid"] = u.first;
		member["room_id"] = u.second.room_id;
		member["broadcasted"] = u.second.broadcasted;
		content += member;
	}
	return content;
}

void BililiveList::Deserialize(const json &content)
{
	for (const auto& p : content.items())
	{
		nlohmann::json member = p.value();
		this->user_list.emplace(member["uid"].get<long>(), info{member["room_id"].get<long>(), member["broadcasted"].get<bool>()});
	}
}

}