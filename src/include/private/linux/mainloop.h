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
 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>

// #include <list>
// #include <functional>
// #include <mutex>
// #include <ostream>
// #include <udjat/request.h>

namespace Udjat {

	namespace Linux {

		class UDJAT_PRIVATE MainLoop : public Udjat::MainLoop {
		private:

			/// @brief Event FD.
			int efd = -1;

			/// @brief Services
			std::list<Service *> services;

		public:
			MainLoop();
			virtual ~MainLoop();

			/// @brief Run mainloop.
			int run() override;

			/// @brief Wakeup main loop.
			void wakeup() noexcept override;

			/// @brief Quit mainloop.
			void quit() override;

			void push_back(Udjat::MainLoop::Service *service) override;
			void remove(Udjat::MainLoop::Service *service) override;

		};

	};

}
