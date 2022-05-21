#ifndef _TRIGGER_BASE_HPP_
#define _TRIGGER_BASE_HPP_

#include <string>
#include <mirai/MiraiBot.hpp>

class ElanorBot;

class TriggerBase
{
protected:
	bool is_running;

public:
	TriggerBase() : is_running(false) {}
	virtual bool TriggerOnStart() { return false; }
	virtual void trigger_on(std::shared_ptr<Cyan::MiraiBot> client, ElanorBot* bot)
	{
		this->is_running = true;
	}

	virtual void trigger_off()
	{
		this->is_running = false;
	}

	virtual bool IsRunning()
	{
		return this->is_running;
	}
	
	virtual ~TriggerBase() 
	{ 
		this->is_running = false;
	}
};

#endif