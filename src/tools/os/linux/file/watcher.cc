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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	std::mutex File::Watcher::guard;

	File::Watcher::Watcher(const Quark &n) : name(n) {
		Controller::getInstance().insert(this);
	}

	File::Watcher::~Watcher() {
		Controller::getInstance().remove(this);
	}

	File::Watcher * File::watch(void *id, const char *name, std::function<void (const Udjat::File::Text &)> callback) {

		Watcher * watcher =  Controller::getInstance().find(name);
		watcher->push_back(id,callback);
		return watcher;

	}

	File::Watcher * File::watch(void *id, const Quark &name, std::function<void (const Udjat::File::Text &)> callback) {

		Watcher * watcher =  Controller::getInstance().find(name.c_str());
		watcher->push_back(id,callback);
		return watcher;

	}

	void File::Watcher::remove(void *id) {
		std::lock_guard<std::mutex> lock(guard);

		files.remove_if([id](const File &file) {
			return file.id == id;
		});

		if(files.size())
			return;

		delete this;
	}

	bool File::Watcher::update(bool force) {

		if(updated && !force) {
			return false;
		}

		try {

#ifdef DEBUG
			cout << "Inotify\tLoading '" << name.c_str() << "' for " << files.size() << " file(s)" << endl;
#endif // DEBUG

			// Open file
			int fd = open(name.c_str(),O_RDONLY);
			if(fd < 0) {
				throw system_error(errno, system_category(), (string{"Can't open '"} + name.c_str() + "'"));
			}

			try {

				struct stat st;
				if(fstat(fd, &st)) {
					throw system_error(errno, system_category(), (string{"Can't get stats of '"} + name.c_str() + "'"));
				}

				if(st.st_mtime == this->mtime) {
#ifdef DEBUG
					cout << "File was not modified" << endl;
#endif // DEBUG
					::close(fd);
					return false;
				}

				this->mtime = st.st_mtime;

				if(files.size()) {

					Udjat::File::Text file(fd,st.st_size);

					for(auto f = files.begin(); f != files.end(); f++) {

						try {

							f->callback(file);

						} catch(const exception &e) {

							cerr << "inotify\tError '" << e.what() << "' updating file '" << this->name.c_str() << "'" << endl;

						}

					}

				}

			} catch(...) {

				::close(fd);
				throw;
			}

			::close(fd);

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

	void File::Watcher::push_back(void *id, std::function<void (const Udjat::File::Text &)> callback) {

		std::lock_guard<std::mutex> lock(guard);
		File file(id,callback);
		files.push_back(file);

	}


 }
