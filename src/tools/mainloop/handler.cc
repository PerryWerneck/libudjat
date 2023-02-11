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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>
 #include <private/misc.h>
 #include <iostream>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	MainLoop::Handler::Handler(int f, const Event e) : fd(f), events(e) {
		MainLoop::getInstance();

#ifdef DEBUG
		cout << "handler\tCreating handler " << hex << ((void *) this) << dec << endl;
#endif // DEBUG
	}

	MainLoop::Handler::~Handler() {
#ifdef DEBUG
		cout << "handler\tDestroying handler " << hex << ((void *) this) << dec << endl;
#endif // DEBUG
		disable();
	}

	bool MainLoop::Handler::enabled() const noexcept {
		return MainLoop::getInstance().enabled(this);
	}

	bool MainLoop::Handler::enable() noexcept {

		MainLoop &mainloop{MainLoop::getInstance()};

		if(mainloop.enabled(this)) {
			return false;
		}
		mainloop.push_back(this);
		return true;
	}

	void MainLoop::Handler::disable() noexcept {
		MainLoop::getInstance().remove(this);
	}

	ssize_t MainLoop::Handler::read(void *buf, size_t count) {
		return ::read(fd,buf,count);
	}

	void MainLoop::Handler::close() {
		if(fd != -1) {
			disable();
			::close(fd);
			fd = -1;
		}
	}

	void MainLoop::Handler::flush() {
	}

	void MainLoop::Handler::set(int fd) {

		if(this->fd != -1) {
			throw system_error(EBUSY,system_category(),"Handler already have a file descriptor");
		}

		this->fd = fd;

	}

	void MainLoop::Handler::set(const Event events) {
		this->events = events;
		if(enabled()) {
			MainLoop::getInstance().wakeup();
		}
	}

 }

