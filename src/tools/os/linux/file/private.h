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
#include <udjat/module/abstract.h>
#include <udjat/tools/file.h>
#include <udjat/tools/quark.h>
#include <udjat/tools/handler.h>
#include <unistd.h>
#include <list>
#include <sys/inotify.h>

using namespace std;

namespace Udjat {

	namespace File {

		class Controller : private MainLoop::Handler {
		private:
			Controller();

			/// @brief Active watches
			list<Watcher *> watchers;

			void onEvent(struct inotify_event *event) noexcept;

		protected:

			void handle_event(const MainLoop::Handler::Event event) override;

		public:
			static Controller & getInstance();
			~Controller();

			Watcher * find(const char *name);
			Watcher * find(const Quark &name);

			void insert(Watcher *watcher);
			void remove(Watcher *watcher);

		};

	}

}
