
#ifndef SERVICE_PRIVATE_H_INCLUDED

	#define SERVICE_PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/defs.h>
	#include <udjat/service.h>
	#include <list>
	#include <mutex>
	#include <iostream>
	#include <system_error>

	using namespace std;
	using Udjat::Service::Module;

	namespace Udjat {

		class Service::Controller {
		private:

#ifdef HAVE_EVENTFD
			int efd;
#endif // HAVE_EVENTFD

			bool enabled;

			recursive_mutex guard;

			Controller();
			void wakeup() noexcept;

			list<Service::Module *> modules;

			struct Timer {
				void *id;
				time_t seconds;			///< @brief Timer interval.
				time_t next;			///< @brief Next Fire.

				const function<bool(const time_t)> call;

				Timer(void *id, time_t seconds, const function<bool(const time_t)> call);

			};

			list<Timer> timers;

			struct Handle {
				void *id;
				int fd;
				Event events;
				const function<bool(const Event event)> call;

				Handle(void *id, int fd, const Event event, const function<bool(const Event event)> call);

			};

			list <Handle> handlers;

		public:
			static Controller & getInstance();
			~Controller();

			void run() noexcept;

			void insert(Service::Module *module);
			void remove(void *id);

			/// @brief Insert socket/file in the list of event sources.
			void insert(void *id, int fd, const Event event, const std::function<bool(const Event event)> call);

			/// @brief Insert timer in the list of event sources.
			void insert(void *id, int seconds, const std::function<bool(const time_t)> call);

		};

	}


#endif // SERVICE_PRIVATE_H_INCLUDED
