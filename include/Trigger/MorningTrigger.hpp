#ifndef	_MORNING_TRIGGER_HPP_
#define _MORNING_TRIGGER_HPP_

#include "CronTriggerBase.hpp"

class MorningTrigger : public CronTriggerBase
{
protected:
	virtual void Action(std::shared_ptr<Cyan::MiraiBot> client, ElanorBot* bot) override;

	virtual bool TriggerOnStart() override { return true; }

	virtual std::string GetCron() override { return "0 0 7 * * *"; }
};

#endif