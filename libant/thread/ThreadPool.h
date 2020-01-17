#ifndef LIBANT_THREADPOOL_H_
#define LIBANT_THREADPOOL_H_

#include <cassert>
#include <atomic>
#include <list>
#include <unordered_set>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "ThreadUtils.h"

namespace ant {

// 线程池。Job实现任务处理流程，需要实现以下接口：
//
//   1. void SetConfig(JobConfig i) 或者 void SetConfig(const JobConfig& i) 或者 void SetConfig(JobConfig&& i)
//      该接口用于设置/重新设置Job需要的配置信息
//
//   2. JobOutput Process(JobInput task)
//      该接口实现任务处理流程
//
// Job的所有成员函数运行在相同的线程，即Job的成员函数之间不存在多线程竞争关系。
template <typename Job, typename JobConfig, typename JobInput, typename JobOutput>
class ThreadPool {
public:
	ThreadPool(int minThreadNum, int maxThreadNum, const JobConfig* cfg = nullptr)
		: maxThreadNum_(maxThreadNum), freeWorkerNum_(0), stop_(false)
	{
		assert(minThreadNum <= maxThreadNum);
		if (cfg) {
			jobCfg_ = *cfg;
		}

		for (int i = 0; i != minThreadNum; ++i) {
			createWorker(true);
		}
	}

	~ThreadPool()
	{
		Stop();
	}

	// 执行任务。如果有空闲线程，直接使用；如果无空闲线程，且未达到最大线程数限制，则新建线程；如果已达到最大限制，则排队等待。
	// 正常情况下，构造函数结束后，freeWorkerNum_应该等于minThreadNum。
	// 极端情况下，构造函数结束后，线程尚未运行，freeWorkerNum_仍然是0。此时，若调用了Run，则会创建新线程。这个新线程可能得不到任务，
	// 然后直接退出；也可能得到任务，执行完毕后再退出（或者继续执行队列中的任务）。
	void Run(JobInput task)
	{
		dataQueueMtx_.lock();
		if (!stop_) {
			if (freeWorkerNum_ <= dataQueue_.size() && allWorkers_.size() < maxThreadNum_) {
				createWorker(false);
			}
			dataQueue_.emplace_back(task);
			dataQueueCond_.notify_one();
		}
		dataQueueMtx_.unlock();
	}

	// 获取任务执行结果
	//   waitMs：等待时间，毫秒。0表示不等待（默认），> 0表示等待毫秒数，< 0表示等到结果为止
	//   返回：pair::second为true表示成功，反之表示失败
	std::pair<JobOutput, bool> GetJobOutput(int waitMs = 0)
	{
		std::unique_lock<std::mutex> lck(dataQueueMtx_);
		if (!stop_) {
			if (waitMs < 0) {
				while (outQueue_.empty()) {
					outQueueCond_.wait(lck);
					if (stop_) {
						break;
					}
				}
			}
			else if (waitMs > 0) {
				if (outQueue_.empty()) {
					outQueueCond_.wait_for(lck, std::chrono::milliseconds(waitMs));
				}
			}
		}

		if (!outQueue_.empty()) {
			auto output = outQueue_.front();
			outQueue_.pop_front();
			return std::make_pair(output, true);
		}
		return std::make_pair(JobOutput(), true);
	}

	// 关闭线程池。该函数会等待所有任务执行完毕后，再退出。
	void Stop()
	{
		dataQueueMtx_.lock();
		if (!stop_) {
			stop_ = true;
			dataQueueMtx_.unlock();
			dataQueueCond_.notify_all();
			outQueueCond_.notify_all();

			// stop_设为true后，只有这里会操作allWorkers_
			for (auto worker : allWorkers_) {
				delete worker;
			}
			allWorkers_.clear();
			freeWorkerNum_ = 0;
		}
		else {
			dataQueueMtx_.unlock();
		}
	}

	void ResetJobConfig(const JobConfig& cfg)
	{
		setJobConfig(cfg);

		dataQueueMtx_.lock();
		if (!stop_) {
			for (auto it = allWorkers_.begin(); it != allWorkers_.end(); ++it) {
				(*it)->ResetJobConfig();
			}
		}
		dataQueueMtx_.unlock();
	}

private:
	class Worker;

private:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void createWorker(bool keepInPool)
	{
		auto worker = new Worker(*this, keepInPool);
		allWorkers_.emplace(worker);
		worker->Start();
	}

	void setJobConfig(const JobConfig& cfg)
	{
		std::lock_guard<std::mutex> lock(jobCfgLock_);
		jobCfg_ = cfg;
	}

	JobConfig getJobConfig() const
	{
		std::lock_guard<std::mutex> lock(jobCfgLock_);
		return jobCfg_;
	}

private:
	mutable std::mutex			jobCfgLock_;
	JobConfig					jobCfg_;

	const size_t				maxThreadNum_;

	std::mutex					dataQueueMtx_;
	std::condition_variable		dataQueueCond_;
	std::list<JobInput>			dataQueue_;
	std::condition_variable		outQueueCond_;
	std::list<JobOutput>		outQueue_;
	std::unordered_set<Worker*>	allWorkers_;
	volatile size_t				freeWorkerNum_;
	volatile bool				stop_;
};

template <typename Job, typename JobConfig, typename JobInput, typename JobOutput>
class ThreadPool<Job, JobConfig, JobInput, JobOutput>::Worker {
public:
	Worker(ThreadPool<Job, JobConfig, JobInput, JobOutput>& thrpool, bool keepInPool = true)
		: pool_(thrpool), reload_(false), keepInPool_(keepInPool), thr_(nullptr)
	{
	}

	~Worker()
	{
		if (thr_) {
			thr_->join();
			delete thr_;
		}
	}

	void Start()
	{
		thr_ = new std::thread(std::bind(&Worker::run, this));
	}

	void ResetJobConfig()
	{
		reload_ = true;
	}

private:
	void run()
	{
		ThreadBlockAllSignals();
		job_.SetConfig(pool_.getJobConfig());

		std::unique_lock<std::mutex> lck(pool_.dataQueueMtx_);
		for (; ; ) {
			while (pool_.dataQueue_.empty()) {
				if (pool_.stop_) {
					lck.unlock();
					return;
				}

				++(pool_.freeWorkerNum_);
				if (keepInPool_) {
					pool_.dataQueueCond_.wait(lck);
				}
				else {
					auto cv = pool_.dataQueueCond_.wait_for(lck, std::chrono::seconds(5));
					if (cv == std::cv_status::timeout) {
						if (!pool_.stop_) { // 极端情况下，可能别的线程调用了pool_.Stop()后，刚好这里也超时，所以需要判断一下
							pool_.allWorkers_.erase(this);
							--(pool_.freeWorkerNum_);
							lck.unlock();
							thr_->detach();
							delete thr_;
							thr_ = nullptr;
							delete this;
						}
						else {
							lck.unlock();
						}
						return;
					}
				}
				--(pool_.freeWorkerNum_);
			}

			auto task = pool_.dataQueue_.front();
			pool_.dataQueue_.pop_front();
			lck.unlock();

			if (needResetJobConfig()) { // 重置配置信息
				job_.SetConfig(pool_.getJobConfig());
			}

			auto result = job_.Process(task);
			lck.lock();
			pool_.outQueue_.emplace_back(result);
			pool_.outQueueCond_.notify_one();
		}
	}

	bool needResetJobConfig()
	{
		return reload_.exchange(false);
	}

private:
	ThreadPool<Job, JobConfig, JobInput, JobOutput>&	pool_;
	std::atomic<bool>									reload_;
	bool												keepInPool_;
	Job													job_;
	std::thread*										thr_;
};

}

#endif // LIBANT_THREADPOOL_H_
