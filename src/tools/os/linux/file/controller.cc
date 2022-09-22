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

 #include "private.h"
 #include <cstring>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>

 #define INOTIFY_EVENT_SIZE ( sizeof (struct inotify_event) )
 #define INOTIFY_EVENT_BUF_LEN ( 1024 * ( INOTIFY_EVENT_SIZE + 16 ) )

 namespace Udjat {

 	File::Controller & File::Controller::getInstance() {
		static Controller instance;
		return instance;
 	}

	File::Controller::Controller() {

		cout << "inotify\tStarting service" << endl;

		MainLoop::Handler::fd = inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
		if(MainLoop::Handler::fd == -1) {
			throw system_error(errno,system_category(),"Can't initialize inotify");
		}

		MainLoop::Handler::events = MainLoop::Handler::oninput;
		MainLoop::Handler::enable();

	}

	File::Controller::~Controller() {

		cout << "inotify\tStopping service" << endl;

		std::lock_guard<std::mutex> lock(Watcher::guard);

		for(auto watcher : watchers) {

			if(watcher->wd != -1) {
				inotify_rm_watch(MainLoop::Handler::fd, watcher->wd);
				watcher->wd = -1;
			}

		}

		MainLoop::Handler::disable();
		::close(MainLoop::Handler::fd);

	}

	void File::Controller::handle_event(const MainLoop::Handler::Event UDJAT_UNUSED(event)) {

		char * buffer = new char[INOTIFY_EVENT_BUF_LEN];
		memset(buffer,0,INOTIFY_EVENT_BUF_LEN);

		ssize_t bytes = read(buffer, INOTIFY_EVENT_BUF_LEN);

		while(bytes > 0) {

			ssize_t	bufPtr	= 0;

			while(bufPtr < bytes) {
				auto pevent = (struct inotify_event *) &buffer[bufPtr];
				onEvent(pevent);
				bufPtr += (offsetof (struct inotify_event, name) + pevent->len);
			}

			bytes = read(buffer, INOTIFY_EVENT_BUF_LEN);
		}

		delete[] buffer;

	}

	File::Watcher * File::Controller::find(const char *name) {

		if(!*name) {
			throw system_error(EBUSY,system_category(),"Empty filename");
		}

		for(auto watcher : watchers) {
			if(!strcmp(watcher->name.c_str(),name)) {
				return watcher;
			}
		}

		// Create a new watcher
		return new Watcher(Quark(name));

	}

	File::Watcher * File::Controller::find(const Quark &name) {

		if(!name) {
			throw system_error(EBUSY,system_category(),"Empty filename");
		}

		for(auto watcher : watchers) {
			if(watcher->name == name) {
				return watcher;
			}
		}

		// Create a new watcher
		return new Watcher(name);

	}

	void File::Controller::insert(Watcher *watcher) {

		if(watcher->wd < 0) {

			watcher->wd = inotify_add_watch(MainLoop::Handler::fd,watcher->name.c_str(),IN_CLOSE_WRITE|IN_DELETE_SELF|IN_MOVE_SELF);
			if(watcher->wd == -1) {
				throw system_error(errno,system_category(),string{"Can't add watch for '"} + watcher->name.c_str() + "'");
			}

			cout << "inotify\tWatching '" << watcher->name.c_str() << "'" << endl;

			watchers.push_back(watcher);

		}

	}

	void File::Controller::remove(Watcher *watcher) {

		if(watcher->wd > 0) {
			if(inotify_rm_watch(MainLoop::Handler::fd, watcher->wd) == -1) {
				cerr << "inotify\tError '" << strerror(errno) << "' unwatching file '"
						<< watcher->name << "' (wd=" << watcher->wd << " instance=" << MainLoop::Handler::fd << ")" << endl;
			} else {
				cerr << "inotify\tUnwatching '" << watcher->name << "'" << endl;
			}
			watcher->wd = -1;
		}

		watchers.remove_if([watcher](const Watcher *w) {
			return watcher == w;
		});

	}

	void File::Controller::onEvent(struct inotify_event *event) noexcept {

		std::lock_guard<std::mutex> lock(Watcher::guard);

		for(auto watcher : watchers) {

			if(watcher->wd == event->wd) {

				// Got event!
				watcher->onEvent(event->mask);
				break;

			}

		}

	}

 }
