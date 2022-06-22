#ifndef _MESSAGE_QUEUE_HPP_
#define _MESSAGE_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
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

	void Start(std::shared_ptr<Cyan::MiraiBot>, std::chrono::milliseconds interval = std::chrono::seconds(1), int max_retry = 3);
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
		int count = 0;
	};

	MessageQueue() {}
	void recv(std::shared_ptr<Cyan::MiraiBot>);

	std::mutex mtx;
	std::condition_variable cv;
	std::thread th;

	std::chrono::milliseconds interval;
	int max_retry;

	bool start = false;
	bool running = false;

	std::queue<GroupMsg> message;
};

#endif