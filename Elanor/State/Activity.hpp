#ifndef _ACTIVITY_HPP_
#define _ACTIVITY_HPP_

#include <State/StateBase.hpp>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>
#include <deque>
#include <mirai/defs/QQType.hpp>

namespace State
{

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

	static constexpr std::string_view _NAME_ = "Activity";

	struct Holder
	{
	private:
		Activity* obj;
		std::string name;
	public:
		Holder(Activity* cd, const std::string& str) : obj(cd), name(str) {}
		Holder(Holder&) = delete;
		~Holder()
		{
			std::lock_guard<std::mutex> lk(obj->mtx);
			if (this->name != obj->ActivityName)
				return;
			obj->hasActivity = false;
			obj->answers.clear();
			obj->cv.notify_all();
		}
	};

	std::unique_ptr<Holder> CheckAndStart(const std::string& name);
	bool HasActivity(void) const;

	std::string GetActivityName(void) const;

	bool WaitForAnswer(AnswerInfo& answer);
	bool WaitForAnswerUntil(std::chrono::time_point<std::chrono::system_clock> due, AnswerInfo& answer);
	
	void AddAnswer(const AnswerInfo& answer);
	
};

}

#endif