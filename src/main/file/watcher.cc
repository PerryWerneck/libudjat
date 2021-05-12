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

 using namespace std;

 namespace Udjat {

	std::mutex File::Watcher::guard;

	File::Watcher::Watcher(const Quark &n) : name(n) {
		Controller::getInstance().insert(this);
	}

	File::Watcher::~Watcher() {
		Controller::getInstance().remove(this);
	}

	File::Watcher * File::Watcher::insert(void *id, const char *name, std::function<void (const char *)> callback) {
		std::lock_guard<std::mutex> lock(guard);

		Watcher * watcher =  Controller::getInstance().find(name);

		Child child(id,callback);
		watcher->children.push_back(child);

		return watcher;
	}

	void File::Watcher::remove(void *id) {
		std::lock_guard<std::mutex> lock(guard);

		children.remove_if([id](const Child &child) {
			return child.id == id;
		});

		if(children.size())
			return;

		delete this;
	}

 }
