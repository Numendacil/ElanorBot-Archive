#ifndef _ACTIVITY_HPP_
#define _ACTIVITY_HPP_

#include <State/StateBase.hpp>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>
#include <deque>
#include <mirai/defs/QQType.hpp>

class Activity : public StateBase
{
public:
	struct AnswerInfo
	{
		std::string answer;
		Cyan::MessageId_t message_id;
	};

protected:
	mutable std::mutex mtx;
	mutable std::condition_variable cv;

	bool hasActivity = false;
	std::string ActivityName;
	std::deque<AnswerInfo> answers;

public:
	struct Holder
	{
	private:
		Activity* obj;
		std::string name;
	public:
		friend class Activity;
		Holder(Activity* cd, const std::string& str) : obj(cd), name(str) {}
		Holder(Holder&) = delete;
		~Holder()
		{
			if (!this->obj) return;
			std::lock_guard<std::mutex> lk(obj->mtx);
			if (this->name != obj->ActivityName)
				return;
			obj->hasActivity = false;
			obj->answers.clear();
			obj->cv.notify_all();
		}
	};

	std::unique_ptr<Holder> CheckAndStart(const std::string& name)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (this->hasActivity)
			return nullptr;
		this->hasActivity = true;
		this->ActivityName = name;
		return std::make_unique<Holder>(this, name);
	}

	bool HasActivity(void) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->hasActivity;
	}

	std::string GetActivityName(void) const
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		return this->ActivityName;
	}

	bool WaitForAnswer(AnswerInfo& answer)
	{
		std::unique_lock<std::mutex> lk(this->mtx);
		cv.wait(lk, [this]{
			return !this->answers.empty() || !this->hasActivity;
		});
		if (!this->hasActivity)
			return false;
		answer = this->answers.front();
		this->answers.pop_front();
		return true;
	}

	bool WaitForAnswerUntil(std::chrono::time_point<std::chrono::system_clock> due, AnswerInfo& answer)
	{
		std::unique_lock<std::mutex> lk(this->mtx);
		if (!cv.wait_until(lk, due, [this]{
			return !this->answers.empty() || !this->hasActivity;
		}))
			return false;
		if (!this->hasActivity)
			return false;
		answer = this->answers.front();
		this->answers.pop_front();
		return true;
	}

	void AddAnswer(const AnswerInfo& answer)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		if (!this->hasActivity)
			return;
		this->answers.push_back(answer);
		cv.notify_all();
	}
};

#endif