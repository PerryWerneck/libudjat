/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
				const char *name;		///< @brief Timer name.
				time_t seconds;			///< @brief Timer interval.
				time_t next;			///< @brief Next Fire.
				time_t running;			///< @brief Is timer running?

				const function<bool(const time_t)> call;

				Timer(void *id, const char *name, const function<bool(const time_t)> call);
				Timer(void *id, const char *name, time_t seconds, const function<bool(const time_t)> call);

			};

			list<Timer> timers;

			struct Handle {
				void *id;
				const char *name;
				int fd;
				Event events;
				time_t running;			///< @brief Is the callback running?

				const function<bool(const Event event)> call;

				Handle(void *id, const char *name, int fd, const Event event, const function<bool(const Event event)> call);

			};

			list <Handle> handlers;

#ifdef HAVE_SIGNAL
			static void onInterruptSignal(int signal) noexcept;
#endif // HAVE_SIGNAL

		public:
			static Controller & getInstance();
			~Controller();

			void run() noexcept;

			void insert(Service::Module *module);
			void remove(void *id);

			/// @brief Insert socket/file in the list of event sources.
			void insert(void *id, const char *name, int fd, const Event event, const std::function<bool(const Event event)> call);

			/// @brief Insert timer in the list of event sources.
			void insert(void *id, const char *name, time_t seconds, const std::function<bool(const time_t)> call);

			/// @brief Insert and emit a timer.
			void insert(void *id, const char *name, const std::function<bool(const time_t)> call);

			/// @brief Set timer.
			/// @param id	Timer ID.
			/// @param time	New alarm time.
			/// @return Old alarm time.
			time_t reset(void *id, time_t seconds, time_t time = 0);

		};

	}


#endif // SERVICE_PRIVATE_H_INCLUDED
