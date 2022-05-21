#ifndef _CRON_TRIGGER_BASE_HPP_
#define _CRON_TRIGGER_BASE_HPP_

#include <mutex>
#include "Timer.hpp"
#include "TriggerBase.hpp"



class CronTriggerBase : public TriggerBase
{
protected:
	mutable std::mutex mtx;

	virtual void Action(std::shared_ptr<Cyan::MiraiBot> client, ElanorBot* bot) = 0;
	size_t timer_id;

	virtual std::string GetCron() { return "0 0 8 * * *"; }

public:
	virtual void trigger_on(std::shared_ptr<Cyan::MiraiBot> client, ElanorBot* bot) override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->is_running)
			return;
		this->is_running = true;
		this->timer_id = Timer::GetInstance().LaunchAt(
			[this, client, bot]
			{
				this->Action(client, bot);
			},
			this->GetCron());
	}

	virtual void trigger_off(void) override
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->is_running)
			return;
		this->is_running = false;
		Timer::GetInstance().Stop(this->timer_id);
	}

	~CronTriggerBase()
	{
		if (this->is_running)
			this->trigger_off();
	}
};

#endif