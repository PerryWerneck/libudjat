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
 #include <udjat/tools/event.h>
 #include <forward_list>

 namespace Udjat {

	class Event::Controller {
	private:
		Controller();

#ifdef _WIN32

		/// @brief Handler for system signal.
		class ConsoleHandlerType : public Udjat::Event  {
		public:
			DWORD dwCtrlType;

			ConsoleHandlerType(DWORD dwCtrlType);
			~ConsoleHandlerType();

			const char * to_string() const noexcept override;

		};

		ConsoleHandlerType & ConsoleHandlerTypeFactory(DWORD dwCtrlType);

		std::forward_list<ConsoleHandlerType> consolehandlertypes;

#else

		/// @brief Handler for system signal.
		class Signal : public Udjat::Event  {
		public:
			int signum;

			Signal(int signum);
			~Signal();

			const char * to_string() const noexcept override;

		};

		Signal & SignalFactory(int signum);

		std::forward_list<Signal> signals;

#endif // _WIN32


	public:
		static std::recursive_mutex guard;

		~Controller();

		static Controller & getInstance();

		void remove(void *id);

#ifdef _WIN32

		Event & ConsoleHandler(void *id, DWORD dwCtrlType, const std::function<bool()> handler);
		static BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType);

		// bool call(DWORD dwCtrlType) noexcept;

#else

		Event & SignalHandler(void *id, int signum, const std::function<bool()> handler);
		static void onSignal(int signum) noexcept;

#endif // _WIN32

	};

 }
