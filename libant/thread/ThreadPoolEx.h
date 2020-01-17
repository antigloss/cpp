#ifndef LIBANT_THREADPOOLEX_H_
#define LIBANT_THREADPOOLEX_H_

#include <cassert>
#include <list>
#include <unordered_set>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ant {

// 线程池。可以执行任意任务，包括普通函数、函数对象、lambda等。任务函数必须返回std::shared_ptr<AbsOutput>
class ThreadPoolEx {
public:
	// 使用该线程池的任务，必须返回std::shared_ptr<AbsOutput>
	class AbsOutput {
	public:
		virtual ~AbsOutput()
		{
		}
	};

private:
	class Worker;

	class AbsExecutor {
	public:
		virtual ~AbsExecutor()
		{
		}
		// 执行任务，把结果插入线程池的结果队列
		virtual void Exec(ThreadPoolEx&) = 0;
	};

	template <typename Function>
	class Executor : public AbsExecutor {
	public:
		Executor(Function&& func) : func_(func)
		{
		}

		virtual void Exec(ThreadPoolEx& pool);

	private:
		Function	func_;
	};

public:
	ThreadPoolEx(int minThreadNum, int maxThreadNum)
		: maxThreadNum_(maxThreadNum), freeWorkerNum_(0), stop_(false)
	{
		assert(minThreadNum <= maxThreadNum);
		for (int i = 0; i != minThreadNum; ++i) {
			createWorker(true);
		}
	}

	~ThreadPoolEx()
	{
		Stop();
	}

	// 执行任务。如果有空闲线程，直接使用；如果无空闲线程，且未达到最大线程数限制，则新建线程；如果已达到最大限制，则排队等待。
	// 正常情况下，构造函数结束后，freeWorkerNum_应该等于minThreadNum。
	// 极端情况下，构造函数结束后，线程尚未运行，freeWorkerNum_仍然是0。此时，若调用了Run，则会创建新线程。这个新线程可能得不到任务，
	// 然后直接退出；也可能得到任务，执行完毕后再退出（或者继续执行队列中的任务）。
	template<typename Function, typename... Args>
	void Run(Function&& task, Args&&... args)
	{
		auto func = std::bind(task, args...);
		auto executor = new Executor<decltype(func)>(move(func));

		taskQueueLock_.lock();
		if (!stop_) {
			if (freeWorkerNum_ <= taskQueue_.size() && allWorkers_.size() < maxThreadNum_) {
				createWorker(false);
			}
			taskQueue_.emplace_back(executor);
			executor = nullptr;
			taskQueueCond_.notify_one();
		}
		taskQueueLock_.unlock();

		delete executor; // delete nullptr是安全的，无需判断
	}

	// 获取任务执行结果
	//   waitMs：等待时间，毫秒。0表示不等待（默认），> 0表示等待毫秒数，< 0表示等到结果为止
	//   返回：pair::second为true表示成功，反之表示失败
	std::shared_ptr<AbsOutput> GetJobOutput(int waitMs = 0);

	// 关闭线程池。该函数会等待所有任务执行完毕后，再退出。
	void Stop();

private:
	ThreadPoolEx(const ThreadPoolEx&) = delete;
	ThreadPoolEx& operator=(const ThreadPoolEx&) = delete;

	void createWorker(bool keepInPool);

	void pushResult(std::shared_ptr<AbsOutput>&& result)
	{
		taskQueueLock_.lock();
		outQueue_.emplace_back(result);
		taskQueueLock_.unlock();
		outQueueCond_.notify_one();
	}

private:
	const size_t							maxThreadNum_;

	std::mutex								taskQueueLock_;
	std::condition_variable					taskQueueCond_;
	std::list<AbsExecutor*>					taskQueue_;
	std::condition_variable					outQueueCond_;
	std::list<std::shared_ptr<AbsOutput>>	outQueue_;
	std::unordered_set<Worker*>				allWorkers_;
	volatile size_t							freeWorkerNum_;
	volatile bool							stop_;
};

template <typename Function>
void ThreadPoolEx::Executor<Function>::Exec(ThreadPoolEx& pool)
{
	pool.pushResult(func_());
}

class ThreadPoolEx::Worker {
public:
	Worker(ThreadPoolEx& thrpool, bool keepInPool = true)
		: pool_(thrpool), keepInPool_(keepInPool), thr_(nullptr)
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

private:
	void run();

private:
	ThreadPoolEx&	pool_;
	bool			keepInPool_;
	std::thread*	thr_;
};

}

#endif // LIBANT_THREADPOOLEX_H_
