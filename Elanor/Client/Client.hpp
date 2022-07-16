#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

#include "mirai/defs/MessageChain.hpp"
#include "mirai/defs/QQType.hpp"
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace Cyan
{
	class MiraiBot;
	class SessionOptions;
}

class Client
{
protected:
	std::unique_ptr<Cyan::MiraiBot> client;
	
	mutable std::mutex q_mtx;
	mutable std::mutex client_mtx;
	mutable std::condition_variable cv;
	std::thread th;

	bool Connected;

	struct Message
	{
		Cyan::GID_t gid;
		Cyan::QQ_t qqid;
		Cyan::MessageChain msg;
		Cyan::MessageId_t mid;
		enum {GROUP, FRIEND, TEMP} type;

		int count = 0;
	};

	std::queue<Message> message;
	std::chrono::milliseconds interval;
	int max_retry;

	Client();
	void MsgQueue();

public:
	Client(const Client&) = delete;
	~Client();

	static Client& GetClient()
	{
		static Client client;
		return client;
	}

	template <typename T>
	Cyan::MiraiBot& On(const std::function<void(T)>& ep);
	void Connect(const Cyan::SessionOptions& opts);
	void Reconnect();
	void Disconnect();
	bool isConnected() { return this->Connected; }

	template<typename F, typename... Args>
	auto Call(F&& f, Args&&... args)
	{
		std::lock_guard<std::mutex> lk(this->client_mtx);
		return std::invoke(std::forward<F>(f), this->client, std::forward<Args>(args)...);
	}

	void Send(const Cyan::GID_t&, const Cyan::MessageChain&, Cyan::MessageId_t = 0);
	void Send(const Cyan::QQ_t&, const Cyan::MessageChain&, Cyan::MessageId_t = 0);
	void Send(const Cyan::GID_t&, const Cyan::QQ_t&, const Cyan::MessageChain&, Cyan::MessageId_t = 0);

};

#endif