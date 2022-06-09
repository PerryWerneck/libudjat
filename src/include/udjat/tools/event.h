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

 #include <udjat/defs.h>
 #include <functional>
 #include <mutex>
 #include <forward_list>

 namespace Udjat {

	/// @brief Generic event.
	class UDJAT_API Event {
	private:

		class Controller;
		friend class Controller;

		struct Listener {
			const void *id;
			const std::function<bool()> handler;
			Listener(const void *i, const std::function<bool()> h) : id(i), handler(h) {
			}
		};

		std::forward_list<Listener> listeners;

	protected:
		Event();

	public:

		Event(Event &src) = delete;
		Event(Event *src) = delete;
		virtual ~Event();

#ifdef WIN32

		/// @brief Get console handler event.
		/// https://docs.microsoft.com/en-us/windows/console/handlerroutine
		static Event & ConsoleHandler(void *id, const char *name, const std::function<bool()> handler);

		/// @brief Get console handler event.
		/// https://docs.microsoft.com/en-us/windows/console/handlerroutine
		/// @param dwCtrlType The type of control signal.
		static Event & ConsoleHandler(void *id, DWORD dwCtrlType, const std::function<bool()> handler);

#else

		/// @brief Insert a signal handler.
		static Event & SignalHandler(void *id, const char *name, const std::function<bool()> handler);

		/// @brief Insert a signal handler.
		static Event & SignalHandler(void *id, int signum, const std::function<bool()> handler);

#endif // !WIN32

		virtual void trigger() noexcept;

		inline void insert(void *id, const std::function<bool()> handler) {
			listeners.emplace_front(id,handler);
		}

		static void remove(void *id);

		inline bool empty() const noexcept {
			return listeners.empty();
		}

		virtual const char * to_string() const noexcept = 0;

	};

 }
