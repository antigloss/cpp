#ifndef LIBANT_FILEDELETER_H_
#define LIBANT_FILEDELETER_H_

#include <string>
#include <vector>
#include <mutex>

class FileDeleter {
public:
	void Start();

	void Add(const std::pair<std::string, time_t>& cfg)
	{
		dirConfigsMtx_.lock();
		dirConfigs_.emplace_back(cfg);
		dirConfigsMtx_.unlock();
	}

	void Clear()
	{
		dirConfigsMtx_.lock();
		dirConfigs_.clear();
		dirConfigsMtx_.unlock();
	}

private:
	typedef	std::vector<std::pair<std::string, time_t>> DirConfigContainer;

private:
	void run();

private:
	std::mutex			dirConfigsMtx_;
	DirConfigContainer	dirConfigs_;
};

#endif // LIBANT_FILEDELETER_H_
