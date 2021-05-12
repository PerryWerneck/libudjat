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
#include <udjat/module.h>
#include <udjat/tools/file.h>
#include <udjat/tools/quark.h>
#include <unistd.h>
#include <list>
#include <sys/inotify.h>

using namespace std;

namespace Udjat {

	class File::Watcher::Controller {
	private:
		Controller();

		/// @brief Inotify instance.
		int instance;

		/// @brief Active watches
		list<Watcher *> watchers;

		void onEvent(struct inotify_event *event) noexcept;

	public:
		static Controller & getInstance();
		~Controller();

		Watcher * find(const char *name);

		void insert(Watcher *watcher);
		void remove(Watcher *watcher);

	};

	/*
	class File::Agent::Controller {
	private:

		/// @brief Mutex to prevent multiple access to file list.
		static recursive_mutex guard;

		/// @brief Inotify instance.
		int instance;

		struct Watch {

			/// @brief File path
			Quark name;

			/// @brief Inotify watch descriptor.
			int wd;

			/// @brief The files was modified?
			int modified;

			/// @brief Files
			list<File::Agent *> files;

			/// @brief Watch has event.
			void onEvent(uint32_t mask) noexcept;

		};

		/// @brief Active watches
		list<Watch> watches;

		Controller();

		void onEvent(struct inotify_event *event) noexcept;

	public:
		~Controller();

		static Controller & getInstance();

		void insert(File::Agent *file);
		void remove(File::Agent *file);

	};
	*/

}
