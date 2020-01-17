#include <csignal>
#include <cerrno>

#include "ThreadUtils.h"

int ThreadBlockAllSignals()
{
#ifndef _WIN32
	sigset_t mask;
	if (sigfillset(&mask) != 0) {
		return errno;
	}

	return pthread_sigmask(SIG_BLOCK, &mask, nullptr);
#else
	return 0;
#endif // !_WIN32
}
