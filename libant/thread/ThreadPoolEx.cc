#include "ThreadUtils.h"
#include "ThreadPoolEx.h"

namespace ant {

std::shared_ptr<ThreadPoolEx::AbsOutput> ThreadPoolEx::GetJobOutput(int waitMs)
{
	std::unique_lock<std::mutex> lck(taskQueueLock_);
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
		return output;
	}
	return std::shared_ptr<AbsOutput>();
}

void ThreadPoolEx::Stop()
{
	taskQueueLock_.lock();
	if (!stop_) {
		stop_ = true;
		taskQueueLock_.unlock();
		taskQueueCond_.notify_all();
		outQueueCond_.notify_all();

		// stop_设为true后，只有这里会操作allWorkers_
		for (auto worker : allWorkers_) {
			delete worker;
		}
		allWorkers_.clear();
		freeWorkerNum_ = 0;
	}
	else {
		taskQueueLock_.unlock();
	}
}

void ThreadPoolEx::createWorker(bool keepInPool)
{
	auto worker = new Worker(*this, keepInPool);
	allWorkers_.emplace(worker);
	worker->Start();
}

void ThreadPoolEx::Worker::run()
{
	ThreadBlockAllSignals();

	std::unique_lock<std::mutex> lck(pool_.taskQueueLock_, std::defer_lock);
	for (; ; ) {
		lck.lock();
		while (pool_.taskQueue_.empty()) {
			if (pool_.stop_) {
				lck.unlock();
				return;
			}

			++(pool_.freeWorkerNum_);
			if (keepInPool_) {
				pool_.taskQueueCond_.wait(lck);
			}
			else {
				auto cv = pool_.taskQueueCond_.wait_for(lck, std::chrono::seconds(5));
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

		auto task = pool_.taskQueue_.front();
		pool_.taskQueue_.pop_front();
		lck.unlock();

		task->Exec(pool_);
		delete task;
	}
}

}