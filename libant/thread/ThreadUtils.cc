#include "ThreadUtils.h"

void ThreadBlockAllSignals()
{
#ifndef _WIN32
	sigset_t mask;
	if (sigfillset(&mask) != 0) {
		int err = errno;
		LOG(WARNING) << "sigfillset failed! err=" << strerror(err);
		return;
	}

	int err = pthread_sigmask(SIG_BLOCK, &mask, nullptr);
	if (err != 0) {
		LOG(WARNING) << "pthread_sigmask failed! err=" << strerror(err);
		return;
	}
#endif // !_WIN32
}
