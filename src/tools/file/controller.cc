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

 	File::Watcher::Controller & File::Watcher::Controller::getInstance() {
		static Controller instance;
		return instance;
 	}

	File::Watcher::Controller::Controller() {

		cout << "inotify\tStarting service" << endl;

		instance = inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
		if(instance == -1) {
			throw system_error(errno,system_category(),"Can't initialize inotify");
		}

		MainLoop::getInstance().insert( (void *) this, instance, MainLoop::oninput, [this](const MainLoop::Event event){

			char * buffer = new char[INOTIFY_EVENT_BUF_LEN];
			memset(buffer,0,INOTIFY_EVENT_BUF_LEN);

			ssize_t bytes = read(instance, buffer, INOTIFY_EVENT_BUF_LEN);

			while(bytes > 0) {

				ssize_t	bufPtr	= 0;

				while(bufPtr < bytes) {
					auto pevent = (struct inotify_event *) &buffer[bufPtr];
					onEvent(pevent);
					bufPtr += (offsetof (struct inotify_event, name) + pevent->len);
				}

				bytes = read(instance, buffer, INOTIFY_EVENT_BUF_LEN);
			}

			delete[] buffer;

			return true;
		});

	}

	File::Watcher::Controller::~Controller() {

		cout << "inotify\tStopping service" << endl;

		std::lock_guard<std::mutex> lock(Watcher::guard);

		for(auto watcher : watchers) {

			if(watcher->wd != -1) {
				inotify_rm_watch(instance, watcher->wd);
				watcher->wd = -1;
			}

		}

		MainLoop::getInstance().remove((void *) this);
		::close(instance);

	}

	File::Watcher * File::Watcher::Controller::find(const char *name) {

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

	File::Watcher * File::Watcher::Controller::find(const Quark &name) {

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

	void File::Watcher::Controller::insert(Watcher *watcher) {

		if(watcher->wd < 0) {

			watcher->wd = inotify_add_watch(instance,watcher->name.c_str(),IN_CLOSE_WRITE|IN_DELETE_SELF|IN_MOVE_SELF);
			if(watcher->wd == -1) {
				throw system_error(errno,system_category(),string{"Can't add watch for '"} + watcher->name.c_str() + "'");
			}

			cout << "inotify\tWatching '" << watcher->name.c_str() << "'" << endl;

			watchers.push_back(watcher);

		}

	}

	void File::Watcher::Controller::remove(Watcher *watcher) {

		if(watcher->wd > 0) {
			if(inotify_rm_watch(instance, watcher->wd) == -1) {
				cerr << "inotify\tError '" << strerror(errno) << "' unwatching file '"
						<< watcher->name << "' (wd=" << watcher->wd << " instance=" << instance << ")" << endl;
			} else {
				cerr << "inotify\tUnwatching '" << watcher->name << "'" << endl;
			}
			watcher->wd = -1;
		}

		watchers.remove_if([watcher](const Watcher *w) {
			return watcher == w;
		});

	}

	void File::Watcher::Controller::onEvent(struct inotify_event *event) noexcept {

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
