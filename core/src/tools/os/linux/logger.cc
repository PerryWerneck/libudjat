
#include <config.h>
#include <udjat/tools/logger.h>
#include <mutex>

using namespace std;

namespace Udjat {

	Logger::Logger(const char *n) : name(n) {
	}

	Logger::Logger(const Atom &n) : name(n) {
	}

	Logger::~Logger() {
	}

	void Logger::redirect(const char *filename, bool console) {

		if(!filename) {
			filename = "/var/log/" PACKAGE_NAME;
		}

	}

	Logger::Writer & Logger::Writer::add(const char *value) {
		args.emplace_back(value);
		return *this;
	}

	Logger::Writer & Logger::Writer::add(const std::string &value) {
		args.push_back(value);
		return *this;
	}

	Logger::Writer & Logger::Writer::add() {
		return *this;
	}

	std::string Logger::Writer::to_string() const {

		string str(this->format);

		auto arg = args.begin();

		size_t pos = str.find("{");
		while(pos != string::npos && arg != args.end()) {

			auto p = str.find("}",pos+1);
			if(p == string::npos)
				break;

			str.replace(pos,(p-pos)+1,*arg);
			arg++;

			pos = str.find("{");
		}

		return str;
	}

}

