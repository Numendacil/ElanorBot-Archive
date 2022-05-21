#ifndef _BILILIVE_TRIGGER_
#define _BILILIVE_TRIGGER_

#include "LoopTriggerBase.hpp"

class BililiveTrigger : public LoopTriggerBase
{
protected:
	virtual void Action(std::shared_ptr<Cyan::MiraiBot> client, ElanorBot* bot) override;

	virtual std::chrono::milliseconds LoopInterval() override { return std::chrono::minutes(5); }
	virtual bool RandStart() { return true; }
};

#endif