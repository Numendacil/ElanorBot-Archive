#pragma once
#ifndef mirai_cpp_events_GroupAllowAnonymousChatEvent_hpp_H_
#define mirai_cpp_events_GroupAllowAnonymousChatEvent_hpp_H_

#include "mirai/third-party/nlohmann/json.hpp"
#include "EventBase.hpp"
#include "mirai/defs/Group.hpp"
#include "mirai/defs/GroupMember.hpp"
#include <optional>
using std::optional;
using std::nullopt;


namespace Cyan
{
	/**
	 * \brief 群匿名聊天权限改变
	 */
	class GroupAllowAnonymousChatEvent : public EventBase
	{
	public:
		bool Origin;
		bool Current;
		Group_t Group;
		std::optional<GroupMember> Operator;

		static MiraiEvent GetMiraiEvent()
		{
			return MiraiEvent::GroupAllowAnonymousChatEvent;
		}

		virtual bool Set(const json& j) override
		{
			this->Origin = j["origin"].get<bool>();
			this->Current = j["current"].get<bool>();
			this->Group.Set(j["group"]);
			if (!j["operator"].is_null())
			{
				GroupMember tmp;
				tmp.Set(j["operator"]);
				Operator = tmp;
			}
			return true;
		}
		virtual json ToJson() const override
		{
			json j = json::object();
			j["type"] = "GroupAllowAnonymousChatEvent";
			j["origin"] = this->Origin;
			j["current"] = this->Current;
			j["group"] = this->Group.ToJson();
			j["operator"] = (Operator != std::nullopt) ? this->Operator->ToJson() : json(nullptr);
			return j;
		}
	};

}

#endif