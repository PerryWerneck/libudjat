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
 #include <vector>

 namespace Udjat {

	/// @brief Generic event.
	class UDJAT_API Event {
	private:

		static std::mutex guard;

		struct Listener {
			void *id;
			const std::function<bool()> handler;
			constexpr Listener(void *i, const std::function<bool()> h) : id(i), handler(h) {
			}
		};

		std::vector<Listener> listeners;

#ifndef WIN32
		static onSignal(int signum) noexcept;
#endif // !WIN32

	protected:
		void trigger() noexcept;

		void Event();
		virtual ~Event();

	public:

#ifndef WIN32

		/// @brief Get event handler for system signal.
		static Event & SignalEventFactory(int signum);

#endif // !WIN32

		void insert(void *id, const std::function<bool()> handler);
		void remove(void *id);


	};

 }

#endif // EVENT_H_INCLUDED
