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

 /**
  * @brief Implements linux file watcher.
  */

 #include <config.h>
 #include <private/filewatcher.h>
 #include <sys/inotify.h>
 #include <udjat/tools/logger.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #define INOTIFY_EVENT_SIZE ( sizeof (inotify_event) )
 #define INOTIFY_EVENT_BUF_LEN ( 1024 * ( INOTIFY_EVENT_SIZE + 16 ) )

 using namespace std;

 namespace Udjat {


	File::Watcher::Controller::Controller() {

		Logger::String{"Starting file watcher"}.trace("services");

		MainLoop::Handler::fd = inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
		if(MainLoop::Handler::fd == -1) {
			throw system_error(errno,system_category(),"Can't initialize inotify");
		}

		MainLoop::Handler::events = MainLoop::Handler::oninput;
		MainLoop::Handler::enable();

	}

	File::Watcher::Controller::~Controller() {

		std::lock_guard<std::mutex> lock(guard);

		/*
		for(auto watcher : watchers) {

			if(watcher->wd != -1) {
				inotify_rm_watch(MainLoop::Handler::fd, watcher->wd);
				watcher->wd = -1;
			}

		}
		*/

		MainLoop::Handler::disable();
		::close(MainLoop::Handler::fd);

	}

	File::Watcher::Controller & File::Watcher::Controller::getInstance() {
		static File::Watcher::Controller instance;
		return instance;
	}

	void File::Watcher::Controller::insert(File::Watcher *watcher) {

		struct stat st;
		if(stat(watcher->pathname, &st)) {
			throw std::system_error(errno,std::system_category(),watcher->pathname);

		}

		if((st.st_mode & S_IFDIR)) {

			// Directory watch
			watch_directory(watcher->pathname,watcher);

		} else if( (st.st_mode & S_IFREG)) {

			// File watch.
			watch_file(watcher->pathname,watcher);

		} else if( (st.st_mode & S_IFLNK)) {

			// Symbolic link
			throw runtime_error(Logger::String{"Unable to watch symlink '",watcher->pathname,"'"});

		} else {

			throw runtime_error(Logger::String{"File '",watcher->pathname,"' has unexpected type"});

		}

	}

	void File::Watcher::Controller::watch_file(const char *path, File::Watcher *watcher) {
	}

	void File::Watcher::Controller::watch_directory(const char *path, File::Watcher *watcher) {
	}

	void File::Watcher::Controller::remove(File::Watcher *watcher) {
	}

	void File::Watcher::Controller::onEvent(const ::inotify_event *event) noexcept {

	}

	void File::Watcher::Controller::handle_event(const MainLoop::Handler::Event) {

		char * buffer = new char[INOTIFY_EVENT_BUF_LEN];
		memset(buffer,0,INOTIFY_EVENT_BUF_LEN);

		ssize_t bytes = read(buffer, INOTIFY_EVENT_BUF_LEN);

		while(bytes > 0) {

			ssize_t	bufPtr	= 0;

			while(bufPtr < bytes) {
				const ::inotify_event *pevent = (::inotify_event *) &buffer[bufPtr];
				onEvent(pevent);
				bufPtr += (offsetof (::inotify_event, name) + pevent->len);
			}

			bytes = read(buffer, INOTIFY_EVENT_BUF_LEN);
		}

		delete[] buffer;

	}

 }

