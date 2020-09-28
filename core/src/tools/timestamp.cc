

#include <cstring>
#include <udjat/timestamp.h>

using namespace std;

std::string Udjat::TimeStamp::to_string(const char *format) const noexcept {
	char timestamp[80];
	memset(timestamp,0,sizeof(timestamp));

	struct tm tm;

	memcpy(&tm,localtime(&value),sizeof(tm));
	strftime(timestamp, 79, format, &tm);

	return std::string(timestamp);
}


