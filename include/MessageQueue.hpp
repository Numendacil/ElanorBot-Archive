#ifndef _MESSAGE_QUEUE_HPP_
#define _MESSAGE_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <mirai/messages/messages.hpp>
#include <mirai/MiraiBot.hpp>

class MessageQueue
{
public:
	MessageQueue(const MessageQueue&) = delete;
	void operator=(const MessageQueue&) = delete;

	static MessageQueue& GetInstance(void)
	{
		static MessageQueue m;
		return m;
	}

	void Start(std::shared_ptr<Cyan::MiraiBot>);
	void Stop(void);
	void Pause(void);
	void Resume(void);

	void Push(const Cyan::GID_t&, const Cyan::MessageChain&, Cyan::MessageId_t = 0);

	~MessageQueue() { this->Stop(); }

private:
	struct GroupMsg
	{
		Cyan::GID_t gid;
		Cyan::MessageChain msg;
		Cyan::MessageId_t mid;
	};

	MessageQueue() {}
	void recv(std::shared_ptr<Cyan::MiraiBot>);

	std::mutex mtx;
	std::condition_variable cv;
	std::thread th;

	bool start = false;
	bool running = false;

	std::queue<GroupMsg> message;
};

#endif