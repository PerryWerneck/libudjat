/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Declares the file watcher controller.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/file/watcher.h>
 #include <udjat/tools/handler.h>
 #include <sys/inotify.h>

 #include <mutex>

 namespace Udjat {

	namespace File {

#ifdef _WIN32

		// TODO: Windows version

#else
		class UDJAT_PRIVATE Watcher::Controller : private MainLoop::Handler {
		private:

			std::mutex guard;

			Controller();
			void onEvent(const struct ::inotify_event *event) noexcept;

			void watch_file(const char *path, File::Watcher *watcher);
			void watch_directory(const char *path, File::Watcher *watcher);

		protected:

			void handle_event(const MainLoop::Handler::Event event) override;

		public:
			static Controller & getInstance();
			~Controller();

			void insert(File::Watcher *watcher);
			void remove(File::Watcher *watcher);

		};

#endif // _WIN32

	}

 }
