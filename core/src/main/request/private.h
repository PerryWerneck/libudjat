#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/request.h>
	#include <udjat/tools/quark.h>
	#include <mutex>
	#include <unordered_map>
	#include <iostream>

	using namespace std;

	namespace Udjat {

		class Request::Controller {
		private:

			static std::recursive_mutex guard;

			// Hash method
			class Hash {
			public:
				inline size_t operator() (const char *a) const {

					// https://stackoverflow.com/questions/7666509/hash-function-for-string
					size_t value = 5381;

					for(const char *ptr = a; *ptr; ptr++) {
						value = ((value << 5) + value) + tolower(*ptr);
					}

					return value;
				}
			};

			// Equal method
			class Equal {
			public:
				inline bool operator() (const char * a, const char * b) const {
					return strcasecmp(a,b) == 0;
				}
			};

			Controller();

			class Method {
			private:
				const std::function<void(Request &request)> method;

			public:
				Method(const std::function<void(Request &request)> m) : method(m) {}

				void call(Request &request) {
					method(request);
				}

			};

			std::unordered_map<const char *, Method, Hash, Equal> methods;

			class JMethod {
			private:
				const std::function<void(const char *path, Json::Value &value)> method;

			public:
				JMethod(const std::function<void(const char *path, Json::Value &value)> m) : method(m) {}

				void call(const char *path, Json::Value &value) {
					method(path,value);
				}

			};

			std::unordered_map<const char *, JMethod, Hash, Equal> jmethods;

		public:
			static Controller & getInstance();
			~Controller();

			void insert(const char *name, std::function<void(Request &request)> method);
			void insert(const char *name, std::function<void(const char *path, Json::Value &value)> method);

			void call(Request &request);
			void call(const char *name, const char *path, Json::Value &value);

		};


	}


#endif // PRIVATE_H_INCLUDED
