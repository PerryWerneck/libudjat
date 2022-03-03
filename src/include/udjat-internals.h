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
 #include <udjat/agent.h>
 #include <functional>
 #include <thread>
 #include <list>

 #ifdef _WIN32
	#define WM_WAKE_UP			WM_USER+100
	#define WM_CHECK_TIMERS		WM_USER+101
	#define WM_STOP				WM_USER+102
	#define WM_EVENT_ACTION		WM_USER+103
	#define IDT_CHECK_TIMERS	1
 #endif // _WIN32

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

	/// @brief Create a root agent.
	/// @return Pointer with a valid root agent.
	UDJAT_PRIVATE std::shared_ptr<Abstract::Agent> RootAgentFactory();
	UDJAT_PRIVATE void setRootAgent(std::shared_ptr<Abstract::Agent> agent);

	/// @brief Load modules from XML definitions.
	UDJAT_PRIVATE void load_modules(const char *pathname);

	/// @brief Get new definition files from server.
	/// @returns Seconds for the next refresh.
	UDJAT_PRIVATE time_t refresh_definitions(const char *pathname);

	/// @brief Load agent definitions from files.
	/// @param agent Root agent.
	/// @param pathname Path for XML agent definitions.
	UDJAT_PRIVATE void load_agent_definitions(std::shared_ptr<Abstract::Agent> agent,const char *pathname);

	class MainLoop::Timer {
	public:

		/// @brief The timer identifier.
		const void *id;

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

