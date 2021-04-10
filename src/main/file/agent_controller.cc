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

 #define INOTIFY_EVENT_SIZE ( sizeof (struct inotify_event) )
 #define INOTIFY_EVENT_BUF_LEN ( 1024 * ( INOTIFY_EVENT_SIZE + 16 ) )

 recursive_mutex Udjat::File::Agent::Controller::guard;

 Udjat::File::Agent::Controller::Controller() {

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

 Udjat::File::Agent::Controller::~Controller() {

	cout << "inotify\tStopping service" << endl;

 	MainLoop::getInstance().remove((void *) this);
	::close(instance);
 }

 Udjat::File::Agent::Controller & Udjat::File::Agent::Controller::getInstance() {
	static Controller instance;
	return instance;
 }

 void  Udjat::File::Agent::Controller::insert(File::Agent *file) {
	lock_guard<std::recursive_mutex> lock(guard);

	// Do we already have this file?
	for(auto watch : watches) {
		if(watch.name == file->name) {
#ifdef DEBUG
			cout << "Already watching '" << file->name << "'" << endl;
#endif // DEBUG
			watch.files.push_back(file);
			return;
		}
	}

	// Create new watch.
	Watch watch;

	watch.name = file->name;
	watch.modified = false;
	watch.files.push_back(file);

	// First, just try to add a new watch.
	watch.wd = inotify_add_watch(instance,watch.name.c_str(),IN_CLOSE_WRITE|IN_MODIFY);
	if(watch.wd == -1) {
		throw system_error(errno,system_category(),string{"Can't add watch for '"} + watch.name.c_str() + "'");
	}

	watches.push_back(watch);
	cout << "inotify\tWatching file '" << file->getName() << "'" << endl;

 }

 void Udjat::File::Agent::Controller::remove(File::Agent *file) {
	lock_guard<std::recursive_mutex> lock(guard);

	watches.remove_if([this, file](Watch &watch){

		watch.files.remove_if([file](File::Agent *agent){
			return agent == file;
		});

		if(!watch.files.empty()) {
			return false;
		}

		// Watch is empty, remove it.
		if(inotify_rm_watch(instance, watch.wd) == -1) {
			cerr << "inotify\tError '" << strerror(errno) << "' unwatching file '" << watch.name << "'" << endl;
		} else {
			cerr << "inotify\tUnwatching '" << watch.name << "'" << endl;
		}

		return true;
	});

 }

 void Udjat::File::Agent::Controller::onEvent(struct inotify_event *event) noexcept {

	for(auto watch = watches.begin(); watch != watches.end(); watch++) {

		if(watch->wd == event->wd) {
			watch->onEvent(event->mask);
			return;
		}

	}

 }

 void Udjat::File::Agent::Controller::Watch::onEvent(uint32_t mask) noexcept {

	if(mask & IN_MODIFY) {
		modified = true;
	}

	if(mask & IN_CLOSE_WRITE) {

		if(modified) {

			modified = false;
			cout << "inotify\tFile '" << name << "' has changes" << endl;

			try {

				File::Local text(name.c_str());

				for(auto file : files) {

					try {
						file->set(text.c_str());
					} catch(const exception &e) {
						cerr << "inotify\t" << name << ": " << e.what() << endl;
					}

				}

			} catch(const exception &e) {

				cerr << "inotify\t" << name << ": " << e.what() << endl;

			}

		}
	}

 }
