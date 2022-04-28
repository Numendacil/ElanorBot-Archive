#include "Timer.hpp"
#include "Common.hpp"

using namespace std;

size_t Timer::GetWorker(void)
{
	this->id_count++;
	for (size_t i = 0; i < this->worker.size(); ++i)
	{
		if (this->worker[i].second.finished)
		{
			this->worker[i].second = {this->id_count, false, false};
			return i;
		}
	}
	this->worker.push_back(make_pair<thread, WorkerState>(thread(), {this->id_count, false, false}));
	return this->worker.size() - 1;
}



size_t Timer::LaunchOnce(function<void()> func, chrono::milliseconds delay)
{
	lock_guard<mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = thread([this, func, delay, idx]
	{
		{
			unique_lock<mutex> lk(this->mtx);
			if (cv.wait_for(lk, delay, [this, idx]{ return this->worker[idx].second.stop;}))
				return;
		}
		func();
		{
			lock_guard<mutex> lk(this->mtx);
			this->worker[idx].second.finished = true;
		}
	});
	return this->worker[idx].second.id;
}



size_t Timer::LaunchLoop(function<void()> func, chrono::milliseconds interval, bool RandStart)
{
	lock_guard<mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = thread([this, func, interval, RandStart, idx]
	{
		if (RandStart)
		{
			std::uniform_real_distribution<float> dist(0, 1);
			auto pre = interval * dist(Common::rng_engine);
			{
				unique_lock<mutex> lk(this->mtx);
				if (cv.wait_for(lk, pre, [this, idx]{ return this->worker[idx].second.stop;}))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}
		while (true)
		{
			func();
			{
				unique_lock<mutex> lk(this->mtx);
				if (cv.wait_for(lk, interval, [this, idx]{ return this->worker[idx].second.stop;}))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}
	});
	return this->worker[idx].second.id;
}



size_t Timer::LaunchLoopPrecise(function<void()> func, chrono::milliseconds interval, bool RandStart)
{
	lock_guard<mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = thread([this, func, interval, RandStart, idx]
	{
		if (RandStart)
		{
			std::uniform_real_distribution<float> dist(0, 1);
			auto pre = interval * dist(Common::rng_engine);
			{
				unique_lock<mutex> lk(this->mtx);
				if (cv.wait_for(lk, pre, [this, idx]{ return this->worker[idx].second.stop; }))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}
		while (true)
		{
			auto t = chrono::steady_clock::now() + interval;
			func();
			{
				unique_lock<mutex> lk(this->mtx);
				if (cv.wait_until(lk, t, [this, idx]{ return this->worker[idx].second.stop;}))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}
	});
	return this->worker[idx].second.id;
}