#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/worker.h>
	#include <udjat/request.h>
	#include <mutex>
	#include <unordered_map>
	#include <iostream>

	using namespace std;

	namespace Udjat {

		class Worker::Controller {
		private:

			static std::recursive_mutex guard;

			Controller();
			~Controller();

			// Hash method
			class Hash {
			public:
				inline size_t operator() (const char * str) const {
					// https://stackoverflow.com/questions/7666509/hash-function-for-string
					size_t value = 5381;

					for(const char *ptr = str; *ptr; ptr++) {
						value = ((value << 5) + value) + tolower(*ptr);
					}

					return value;
				}
			};

			// Equal method
			class Equal {
			public:
				inline bool operator() (const char *a, const char *b) const {
					return strcasecmp(a,b) == 0;
				}
			};

			std::unordered_map<const char *, const Worker *, Hash, Equal> workers;

		public:
			static Controller & getInstance();

			void insert(const Worker *worker);
			void remove(const Worker *worker);

			const Worker * find(const char *name) const;
		};

	}


#endif // PRIVATE_H_INCLUDED
