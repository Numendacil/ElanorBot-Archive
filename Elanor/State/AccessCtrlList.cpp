#include "AccessCtrlList.hpp"
#include <ThirdParty/json.hpp>

using json = nlohmann::json;

namespace State
{

json AccessCtrlList::Serialize()
{
	json content;
	for (const auto& p : this->WhiteList)
		content["WhiteList"] += p.ToInt64();
	for (const auto& p : this->BlackList)
		content["BlackList"] += p.ToInt64();
	return content;
}

void AccessCtrlList::Deserialize(const json &content)
{
	if (content.contains("WhiteList"))
	{
		if (content["WhiteList"].type() == json::value_t::array)
		{
			for (const auto& p : content["WhiteList"].items())
				this->WhiteList.insert((Cyan::QQ_t)p.value());
		}
	}
	if (content.contains("BlackList"))
	{
		if (content["BlackList"].type() == json::value_t::array)
		{
			for (const auto& p : content["BlackList"].items())
				this->BlackList.insert((Cyan::QQ_t)p.value());
		}
	}
}

}