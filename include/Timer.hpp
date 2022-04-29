#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <functional>

class Timer
{
public:
	Timer(const Timer&) = delete;
	void operator=(const Timer&) = delete;

	static Timer& GetInstance(void)
	{
		static Timer t;
		return t;
	}

	size_t LaunchOnce(std::function<void()>, std::chrono::milliseconds);
	size_t LaunchLoop(std::function<void()>, std::chrono::milliseconds, bool = false);
	size_t LaunchLoopPrecise(std::function<void()>, std::chrono::milliseconds, bool = false);

	void Stop(size_t id)
	{
		std::lock_guard<std::mutex> lk(this->mtx);
		for (auto& p : this->worker)
		{
			if (p.second.id == id)
				p.second.stop = true;
		}
		cv.notify_all();
	}


	~Timer()
	{
		for (auto& p : this->worker)
			p.second.stop = true;
		cv.notify_all();
		for (auto& p : this->worker)
			if (p.first.joinable())
				p.first.join();
	}

private:
	Timer() {};
	size_t GetWorker(void);

	struct WorkerState
	{
		size_t id;
		bool finished = true;
		bool stop = false;
	};

	std::vector<std::pair<std::thread, WorkerState>> worker;
	std::mutex mtx;
	std::condition_variable cv;
	size_t id_count = 0;
};

#endif