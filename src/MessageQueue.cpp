#include <mirai/exceptions/exceptions.hpp>
#include <chrono>
#include "MessageQueue.hpp"
#include "utils/log.h"
using namespace Cyan;
using namespace std;

void MessageQueue::recv(shared_ptr<MiraiBot> bot)
{
	while(true)
	{
		GroupMsg msg;
		{
			unique_lock<mutex> lk(this->mtx);
			this->cv.wait(lk, [this]() -> bool
			{ 
				if (!this->start)
					return true;
				return this->running && !this->message.empty();
			});
			if (!this->start)
				return;
			msg = this->message.front();
		}

		try
		{
			this_thread::sleep_for(std::chrono::seconds(1));
			bot->SendMessage(msg.gid, msg.msg, msg.mid);
			this->message.pop();
		}
		catch (runtime_error& e)
		{
			logging::WARN(string("MessageQueue: ") + e.what());
		}
	}
}

void MessageQueue::Start(shared_ptr<MiraiBot> bot)
{
	assert(!this->th.joinable());
	{
		lock_guard<mutex> lk(this->mtx);
		this->start = true;
		this->running = true;
	}
	this->th = thread(&MessageQueue::recv, this, bot);
}

void MessageQueue::Stop(void)
{
	{
		lock_guard<mutex> lk(this->mtx);
		this->start = false;
	}
	this->cv.notify_all();
	if (this->th.joinable())
		this->th.join();
	{
		lock_guard<mutex> lk(this->mtx);
		this->running = false;
	}
}

void MessageQueue::Resume(void)
{
	lock_guard<mutex> lk(this->mtx);
	this->running = true;
	this->cv.notify_all();
}

void MessageQueue::Pause(void)
{
	lock_guard<mutex> lk(this->mtx);
	this->running = false;
}


void MessageQueue::Push(const GID_t& target, const MessageChain& messageChain, MessageId_t msgId)
{
	GroupMsg msg{target, messageChain, msgId};
	lock_guard<mutex> lk(this->mtx);
	this->message.push(msg);
	this->cv.notify_all();
}