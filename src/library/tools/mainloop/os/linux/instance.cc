/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/linux/mainloop.h>
 #include <mutex>
 #include <dlfcn.h>

 #ifdef DEBUG
	#include <private/glib/mainloop.h>
 #endif // DEBUG
 
 using namespace std;

 namespace Udjat {

	MainLoop & MainLoop::getInstance() {

		static mutex guard;
		lock_guard<mutex> lock(guard);

		if(!instance) {

#ifdef DEBUG
			if(Glib::MainLoop::available()) {
				static Glib::MainLoop inst;
				return inst;
			}
#endif 

			static Linux::MainLoop inst;
			return inst;
		}

		return *instance;
	}

 }