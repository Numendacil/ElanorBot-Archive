#ifndef _LOOP_TRIGGER_BASE_HPP_
#define _LOOP_TRIGGER_BASE_HPP_

#include <chrono>
#include <mutex>
#include "Timer.hpp"
#include "TriggerBase.hpp"



class LoopTriggerBase : public TriggerBase
{
protected:
	mutable std::mutex mtx;

	virtual void Action(std::shared_ptr<Cyan::MiraiBot> client, ElanorBot* bot) = 0;
	size_t timer_id;

	virtual std::chrono::milliseconds LoopInterval() { return std::chrono::hours(1); }
	virtual bool RandStart() { return false; }

public:
	virtual void trigger_on(std::shared_ptr<Cyan::MiraiBot> client, ElanorBot* bot) override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->is_running)
			return;
		this->is_running = true;
		this->timer_id = Timer::GetInstance().LaunchLoop(
			[this, client, bot]
			{
				this->Action(client, bot);
			},
			this->LoopInterval(),
			this->RandStart());
	}

	virtual void trigger_off(void) override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->is_running)
			return;
		this->is_running = false;
		Timer::GetInstance().Stop(this->timer_id);
	}

	~LoopTriggerBase()
	{
		if (this->is_running)
			this->trigger_off();
	}
};

#endif