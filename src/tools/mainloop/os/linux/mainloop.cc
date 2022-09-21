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
 #include <cstring>
 #include <sys/eventfd.h>
 #include <private/misc.h>
 #include <iostream>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	MainLoop::MainLoop() {
		cout << "MainLoop\tInitializing service loop" << endl;
		efd = eventfd(0,0);
		if(efd < 0)
			throw system_error(errno,system_category(),"eventfd() has failed");

	}

	MainLoop::~MainLoop() {

#ifdef DEBUG
		cout << "MainLoop\tDestroying mainloop" << endl;
#endif // DEBUG

		if(!handlers.empty()) {

			cerr << "MainLoop\tDestroying mainloop with " << handlers.size() << " pending handler(s)" << endl;

		} else {
			cout << "MainLoop\tDestroying clean service loop" << endl;
		}

		enabled = false;
		wakeup();

		{
			lock_guard<mutex> lock(guard);
			::close(efd);
			efd = -1;
		}

#ifdef DEBUG
		cout << "MainLoop\tMainloop was destroyed" << endl;
#endif // DEBUG

	}

	void MainLoop::wakeup() noexcept {
		if(efd != -1) {
			static uint64_t evNum = 0;
			if(write(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
				cerr << "MainLoop\tError '" << strerror(errno) << "' writing to event loop using fd " << efd << endl;
			}
			evNum++;
		}
#ifdef DEBUG
		else {
			cerr << "MainLoop\t" << __FILE__ << "(" << __LINE__ << "): Unexpected call with efd=" << efd << endl;
		}
#endif // DEBUG
	}

 }
