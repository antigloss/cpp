#include <thread>
#include <boost/filesystem.hpp>

#ifndef _WIN32
#include <signal.h>
#endif

#include "FileDeleter.h"

using namespace std;
namespace fs = boost::filesystem;

void FileDeleter::Start()
{
	thread t(bind(&FileDeleter::run, this));
	t.detach();
}

void FileDeleter::run()
{
#ifndef _WIN32
	// 屏蔽信号
	sigset_t mask;
	if (sigfillset(&mask) == 0) {
		pthread_sigmask(SIG_BLOCK, &mask, nullptr);
	}
#endif // !_WIN32

	string tmpBuf;
	DirConfigContainer cfgs;
	for (;;) {
		dirConfigsMtx_.lock();
		cfgs = dirConfigs_;
		dirConfigsMtx_.unlock();
		
		for (const auto& cfg : cfgs) {
			fs::path p(cfg.first);
			try {
				time_t tNow = time(0);
				for (const auto& entry : fs::directory_iterator(p)) {
					if (tNow - fs::last_write_time(entry) > cfg.second) {
						if (fs::is_regular_file(entry)) {
							fs::remove(entry);
						} else if (fs::is_directory(entry)) {
							fs::remove_all(entry);
						}
					}
				}
			} catch (...) {
				// nothing to do
			}			
		}

		this_thread::sleep_for(chrono::seconds(300)); // 每隔五分钟扫描一遍
	}
}
