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

		static std::mutex guard;

		struct Listener {
			const void *id;
			const std::function<bool()> handler;
			Listener(const void *i, const std::function<bool()> h) : id(i), handler(h) {
			}
		};

		std::forward_list<Listener> listeners;

	protected:
		Event();
		virtual ~Event();

	public:

		Event(const Event &src) = delete;
		Event(const Event *src) = delete;

#ifndef WIN32

		/// @brief Get event handler for system signal.
		static Event & SignalEventFactory(int signum);

#endif // !WIN32

		void trigger() noexcept;

		/// @brief Insert event handler.
		/// @param id The event handler id.
		/// @param handler The event handler; will be removed on exception or 'false' return.
		void insert(void *id, const std::function<bool()> handler);
		void remove(void *id);

		virtual std::string to_string() const noexcept;


	};

 }

