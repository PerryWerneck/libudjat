#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/worker.h>
	#include <udjat/request.h>
	#include <mutex>
	#include <unordered_set>

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
				inline size_t operator() (const Worker * worker) const {
					return worker->hash();
				}
			};

			// Equal method
			class Equal {
			public:
				inline bool operator() (const Worker *a, const Worker *b) const {
					return strcasecmp(a->c_str(),b->c_str());
				}
			};

			std::unordered_set<Worker *, Hash, Equal> workers;

		public:
			static Controller & getInstance();

			void insert(Worker *worker);
			void remove(Worker *worker);

		};

	}


#endif // PRIVATE_H_INCLUDED
