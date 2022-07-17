#ifndef	_MORNING_TRIGGER_HPP_
#define _MORNING_TRIGGER_HPP_

#include <Trigger/CronTriggerBase.hpp>

namespace Trigger
{

class MorningTrigger : public CronTriggerBase
{
protected:
	virtual void Action() override;

	virtual bool TriggerOnStart() override { return true; }

	virtual std::string GetCron() override { return "0 0 7 * * *"; }
};

}

#endif