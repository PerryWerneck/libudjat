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

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <functional>
 #include <thread>
 #include <list>

 #ifdef HAVE_LIBINTL
	#include <locale.h>
	#include <libintl.h>
	#define _( x )                  dgettext(PACKAGE_NAME,x)
	#define N_( x )                 x
 #else
	#define _( x )                  x
	#define N_( x )                 x
 #endif // HAVE_LIBINTL

 namespace Udjat {

	class MainLoop::Timer {
	public:

		/// @brief The timer identifier.
		const void *id;

		/// @brief Is the timer running.
		bool running = false;

		/// @brief The interval in milliseconds.
		unsigned long interval;

		/// @brief The interval in milliseconds.
		unsigned long next;

		/// @brief The timer method.
		const std::function<bool()> call;

		/// @brief Get current timer.
		static unsigned long getCurrentTime();

		/// @brief Create timer.
		Timer(const void *id, unsigned long milliseconds, const std::function<bool()> call);

		/// @brief Reset timer.
		void reset(unsigned long milliseconds);

	};

	class MainLoop::Handler {
	public:

		const void *id;
		int fd;
		Event events;
		time_t running;			///< @brief Is the callback running?

		const std::function<bool(const Event event)> call;

		Handler(const void *id, int fd, const Event event, const std::function<bool(const Event event)> call);

	};

	#ifdef _WIN32
	class ThreadPool::Controller : MainLoop::Service {
	private:

		std::mutex guard;
		HWND hwnd;

		std::list<ThreadPool *> pools;

		Controller();
		~Controller() { }

	public:
		static Controller &getInstance();

		inline void push_back(ThreadPool *pool) {
			std::lock_guard<std::mutex> lock(guard);
			pools.push_back(pool);
		}

		inline void remove(ThreadPool *pool) {
			std::lock_guard<std::mutex> lock(guard);
			pools.remove(pool);
		}

		void stop() override;

	};
	#endif // _WIN32

 }

