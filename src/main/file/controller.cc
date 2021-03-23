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
 #include <sys/inotify.h>

 recursive_mutex Udjat::File::Controller::guard;

 Udjat::File::Controller::Controller() {

	instance = inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
	if(instance == -1) {
		throw system_error(errno,system_category(),"Cant initialize inotify");
	}

 }

 Udjat::File::Controller::~Controller() {
	::close(instance);
 }

 Udjat::File::Controller & Udjat::File::Controller::getInstance() {
	static Controller instance;
	return instance;
 }

 void  Udjat::File::Controller::insert(File *file) {
	lock_guard<std::recursive_mutex> lock(guard);

	// Do we already have this file?
	for(auto watch : watches) {
		if(watch.name == file->name) {
#ifdef DEBUG
			cout << "inotify\tInserting file " << file->name << endl;
#endif // DEBUG
			watch.files.push_back(file);
			return;
		}
	}

	// Create new watch.
	Watch watch{file->name};

	watch.name = file->name;
	watch.files.push_back(file);

	// First, just try to add a new watch.
	watch.wd = inotify_add_watch(instance,watch.name.c_str(),IN_CLOSE_WRITE|IN_MODIFY);
	if(watch.wd != -1) {
		watches.push_back(watch);
		return;
	}

	if(errno != ENOENT) {
		throw system_error(errno,system_category(),watch.name.c_str());
	}

	// File not found, watch path.
	cerr << "inotify\tWatching file creation is not supported" << endl;
	throw system_error(ENOENT,system_category(),watch.name.c_str());

 }

 void  Udjat::File::Controller::remove(File *file) {
	lock_guard<std::recursive_mutex> lock(guard);
 }

