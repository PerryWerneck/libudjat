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
 #include <unistd.h>
 #include <cstring>
 #include <udjat/tools/threadpool.h>
 #include <sched.h>

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

	File::Watcher * File::Watcher::insert(void *id, const Quark &name, std::function<void (const char *)> callback) {
		std::lock_guard<std::mutex> lock(guard);

		Watcher * watcher =  Controller::getInstance().find(name.c_str());

		Child child(id,callback);
		watcher->children.push_back(child);
		watcher->updated = false;

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

	bool File::Watcher::update(bool force) {

		if(updated && !force) {
			return false;
		}

		try {

#ifdef DEBUG
			cout << "Inotify\tLoading '" << name.c_str() << "' for " << children.size() << " children" << endl;
#endif // DEBUG

			File::Local file(name.c_str());

			for(auto child = children.begin(); child != children.end(); child++) {

				try {

					child->callback(file.c_str());

				} catch(const exception &e) {

					cerr << "inotify\tError '" << e.what() << "' updating file '" << this->name.c_str() << "'" << endl;

				}

			}

		} catch(const exception &e) {

			cerr << "inotify\tError '" << e.what() << "' loading '" << this->name.c_str() << "'" << endl;

		}

		return updated = true;

	}

	void File::Watcher::onChanged() noexcept {

		Controller::getInstance().remove(this);

		ThreadPool::getInstance().push([this]() {

			sched_yield();

			if(::access(this->name.c_str(),F_OK) == -1) {

				cerr << "inotify\tFile '" << name.c_str() << "': " << strerror(errno) << endl;

			} else {

				update(true);
				Controller::getInstance().insert(this);

			}

		});

	}

	void File::Watcher::onEvent(const uint32_t event) noexcept {

		if(event & IN_IGNORED) {

			// Watch  was  removed  explicitly or automatically (file  was  deleted, or filesystem was unmounted)
			cout << "inotify\tFile '" << this->name.c_str() << "' was ignored" << endl;
			this->wd = -1;
			onChanged();
		}

		if(event & IN_CLOSE_WRITE) {
			cout << "inotify\tFile '" << this->name.c_str() << "' was changed" << endl;
			ThreadPool::getInstance().push([this]() {
				sched_yield();
				update(true);
				Controller::getInstance().insert(this);
			});
		}

		if(event & IN_DELETE_SELF) {
			cout << "inotify\tFile '" << this->name.c_str() << "' was deleted" << endl;
		}

		if(event & IN_MOVE_SELF) {
			cout << "inotify\tFile '" << this->name.c_str() << "' was moved" << endl;
			onChanged();
		}

	}

 }
