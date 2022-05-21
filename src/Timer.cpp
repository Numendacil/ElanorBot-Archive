#include "Timer.hpp"
#include "Common.hpp"
#include "utils/croncpp.h"
#include "utils/log.h"

using namespace std;

size_t Timer::GetWorker(void)
{
	this->id_count++;
	for (size_t i = 0; i < this->worker.size(); ++i)
	{
		if (this->worker[i].second.finished)
		{
			if (this->worker[i].first.joinable())
				this->worker[i].first.join();
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

		try
		{
			func();
		}
		catch (const exception& e)
		{
			logging::WARN("Exception occured <Timer::LaunchOnce>: " + string(e.what()));
		}

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
			try
			{
				func();
			}
			catch (const exception &e)
			{
				logging::WARN("Exception occured <Timer::LaunchLoop>: " + string(e.what()));
			}

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



size_t Timer::LaunchAt(function<void()> func, const string& cron_str, int num)
{
	auto crontab = cron::make_cron(cron_str);
	lock_guard<mutex> lk(this->mtx);
	size_t idx = this->GetWorker();
	this->worker[idx].first = thread([this, func, crontab, num, idx]
	{
		int count = 0;
		while (true)
		{
			auto t = cron::cron_next(crontab, chrono::system_clock::now());
			{
				unique_lock<mutex> lk(this->mtx);
				if (cv.wait_until(lk, t, [this, idx]{ return this->worker[idx].second.stop;}))
				{
					this->worker[idx].second.finished = true;
					return;
				}
			}

			try
			{
				func();
			}
			catch (const exception &e)
			{
				logging::WARN("Exception occured <Timer::LaunchAt>: " + string(e.what()));
			}

			if (num > 0)
			{
				count++;
				if (count >= num)
				{
					unique_lock<mutex> lk(this->mtx);
					this->worker[idx].second.finished = true;
					return;
				}
			}
		}
	});
	return this->worker[idx].second.id;
}