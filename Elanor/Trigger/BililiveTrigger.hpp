#ifndef _BILILIVE_TRIGGER_
#define _BILILIVE_TRIGGER_

#include <Trigger/LoopTriggerBase.hpp>

namespace Trigger
{

class BililiveTrigger : public LoopTriggerBase
{
protected:
	virtual void Action() override;

	virtual std::chrono::milliseconds LoopInterval() override { return std::chrono::minutes(5); }
	virtual bool RandStart() override { return true; }
};

}

#endif