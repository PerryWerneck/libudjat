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
 #include <list>

 #ifdef _WIN32
	#include <udjat/win32/handler.h>
 #endif // _WIN32

 #include <mutex>

 namespace Udjat {

	namespace File {

#ifdef _WIN32

		// TODO: Windows version
		class UDJAT_PRIVATE Watcher::Controller {
		private:

			class Handler : private Win32::Handler {
			public:
				File::Watcher *file;
				DWORD dwNotifyFilter;

				Handler(File::Watcher *f,DWORD dw, HANDLE handle) : Win32::Handler{handle}, file{f}, dwNotifyFilter{dw} {
					enable();
				}

				void handle(bool abandoned) override;

				void close() override {
					if(hEvent) {
						FindCloseChangeNotification(hEvent);
						Win32::Handler::close();
					}
				}

			};

			std::list<Handler> handlers;
			std::mutex guard;

			void add_watch(File::Watcher *watcher, const char *path, BOOL bWatchSubtree, DWORD dwNotifyFilter);
			void watch_file(File::Watcher *watcher);
			void watch_directory(File::Watcher *watcher);

			Controller();

		public:
			static Controller & getInstance();
			~Controller();

			void insert(File::Watcher *watcher);
			void remove(File::Watcher *watcher);

		};
#else
		class UDJAT_PRIVATE Watcher::Controller : private MainLoop::Handler {
		private:

			/// @brief A watch handler.
			struct Handler {
				int wd = -1;
				std::list<File::Watcher *> files;
			};

			std::list<Handler> handlers;
			std::mutex guard;

			Controller();
			void onEvent(const struct ::inotify_event *event) noexcept;

			void watch_file(File::Watcher *watcher);
			void watch_directory(File::Watcher *watcher);

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
