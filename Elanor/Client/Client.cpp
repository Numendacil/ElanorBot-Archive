#include "Client.hpp"
#include "mirai/defs/QQType.hpp"
#include <chrono>
#include <mirai.h>
#include <memory>
#include <stdexcept>
#include <thread>

using namespace std;

Client::Client()
{
	this->Connected = false;
	this->interval = chrono::milliseconds(500);
	this->max_retry = 3;
	this->client = make_unique<Cyan::MiraiBot>();
}

void Client::MsgQueue()
{
	while(true)
	{
		Message msg;
		{
			unique_lock<mutex> lk(this->mtx);
			this->cv.wait(lk, [this]() -> bool
			{ 
				if (!this->Connected)
					return true;
				return !this->message.empty();
			});
			if (!this->Connected)
				return;
			msg = move(this->message.front());
			this->message.pop();
		}

		try
		{
			this_thread::sleep_for(this->interval);
			switch (msg.type)
			{
			case Message::GROUP:
				this->client->SendMessage(msg.gid, msg.msg, msg.mid);
				break;
			case Message::FRIEND:
				this->client->SendMessage(msg.qqid, msg.msg, msg.mid);
				break;
			case Message::TEMP:
				this->client->SendMessage(msg.gid, msg.qqid, msg.msg, msg.mid);
				break;
			default:
				;	// logging::ERROR("waht");
			}
		}
		catch(runtime_error& e)
		{
			// logging::WARN(string("MsgQueue: ") + e.what());
			if (msg.count < this->max_retry)
			{
				unique_lock<mutex> lk(this->mtx);
				msg.count++;
				this->message.push(move(msg));
			}
		}
	}
}

void Client::Send(const Cyan::GID_t& gid, const Cyan::MessageChain& msg, Cyan::MessageId_t mid)
{
	unique_lock<mutex> lk(this->mtx);
	this->message.emplace(gid, 0, msg, mid, Message::GROUP);
}

void Client::Send(const Cyan::QQ_t& qqid, const Cyan::MessageChain& msg, Cyan::MessageId_t mid)
{
	unique_lock<mutex> lk(this->mtx);
	this->message.emplace(0, qqid, msg, mid, Message::FRIEND);
}

void Client::Send(const Cyan::GID_t& gid, const Cyan::QQ_t& qqid, const Cyan::MessageChain& msg, Cyan::MessageId_t mid)
{
	unique_lock<mutex> lk(this->mtx);
	this->message.emplace(gid, qqid, msg, mid, Message::TEMP);
}



template <typename T>
Cyan::MiraiBot& Client::On(const std::function<void(T)>& ep)
{
	return this->client->On<T>(ep);
}

void Client::Connect(const Cyan::SessionOptions &opts)
{
	this->client->Connect(opts);
	this->Connected = true;
	this->th = thread(&Client::MsgQueue, this);
}

void Client::Reconnect()
{
	this->Connected = false;
	if (this->th.joinable())
		this->th.join();
	
	this->client->Reconnect();
	this->Connected = true;
	this->th = thread(&Client::MsgQueue, this);
}

void Client::Disconnect()
{
	this->Connected = false;
	if (this->th.joinable())
		this->th.join();
	this->client->Disconnect();
}

Cyan::MiraiBot& Client::GetMiraiBot()
{
	return *this->client.get();
}