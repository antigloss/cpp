#ifndef LIBANT_TIME_UTILS_H_
#define LIBANT_TIME_UTILS_H_

#include <ctime>

inline time_t ToUnixTime(int year, int mon, int day, int hour, int min, int sec)
{
	tm tm;
	tm.tm_isdst = 0;
	tm.tm_year = year - 1900;
	tm.tm_mon = mon - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;
	return mktime(&tm);
}

#endif // LIBANT_TIME_UTILS_H_
