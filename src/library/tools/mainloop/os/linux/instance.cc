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
 #include <udjat/tools/logger.h>
 #include <mutex>
 #include <dlfcn.h>
 #include <private/glib/mainloop.h>
 
 using namespace std;

 namespace Udjat {

	MainLoop & MainLoop::getInstance() {

		static mutex guard;
		lock_guard<mutex> lock(guard);

		if(!instance) {

			if(Glib::MainLoop::available()) {
				static Glib::MainLoop inst;
				debug("Using GLib main loop - ", (int) inst.type() == MainLoop::GLib ? "GLib" : "Unknown");
				Logger::String{"GLib mainloop is available"}.write(Logger::Debug);
				return inst;
			}
			static Linux::MainLoop inst;
			return inst;
		}

		debug("Returning existing main loop - ", (int) instance->type() == MainLoop::GLib ? "GLib" : "Unknown");
		return *instance;
	}

 }