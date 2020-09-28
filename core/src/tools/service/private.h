/*
 *
 * Copyright (C) <2019> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 *
 * @file
 *
 * @brief
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 *
 */

#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	/*
	#include <udjat/defs.h>
	#include <udjat/string.h>
	#include <udjat/service.h>
	#include <list>
	#include <functional>
	#include <mutex>

	using std::list;
	using std::mutex;
	using std::lock_guard;
	using std::system_error;
	using std::system_category;

	namespace Udjat::Service {

		class DLL_PRIVATE Controller {
		private:

			/// @brief Guard lock for the controller.
			mutex mtx;

			/// @brief Is the controller active?
			bool enabled;

			struct Handler {
				int fd;														///< @brief Socket to handle.
				Service::Event ev;											///< @brief Events to handle.
				std::function<bool(int fd, enum Event ev)> callback;		///< @brief Callbacks method.
			};

			/// @brief Socket list.
			list<Handler> handlers;

			/// @brief Timer list.
			list<Abstract::Timer *> timers;

			/// @brief Event listeners.
			list<Abstract::EventListener *> listeners;

			/// @brief FDEvent (for wake up)
			int fdevent;

			/// @brief Wake up main loop
			void wakeup();

		public:

			static void terminate(int signal) noexcept;
			static void reload(int signal) noexcept;

			static Controller& getInstance();

			Controller();
			~Controller();

			/// @brief Run main loop.
			void run();

			/// @brief Insert timer.
			void insert(Abstract::Timer *timer);

			/// @brief Remove timer.
			void remove(Abstract::Timer *timer);

			void push_back(Abstract::EventListener *listener);
			void remove(Abstract::EventListener *listener);

		};


	}
	*/

#endif // PRIVATE_H_INCLUDED
