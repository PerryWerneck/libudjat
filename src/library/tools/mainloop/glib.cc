/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/glib/mainloop.h>
 #include <dlfcn.h>

 namespace Udjat {

	enum methods {
		METHOD_G_SOURCE_NEW,
		METHOD_G_SOURCE_ATTACH,
		METHOD_G_SOURCE_ADD_POLL,
		METHOD_G_SOURCE_DESTROY,
		METHOD_G_TIMEOUT_ADD_FULL
	};

	static const char *required_methods[] = {
		"g_source_new",
		"g_source_attach",
		"g_source_add_poll",
		"g_source_destroy",
		"g_timeout_add_full"
	};

	static void *methods[sizeof(required_methods)/sizeof(required_methods[0])];

	bool Glib::MainLoop::available() noexcept {

		for(size_t ix = 0; ix < sizeof(required_methods)/sizeof(required_methods[0]); ix++) {

			dlerror();
			methods[ix] = dlsym(RTLD_DEFAULT, required_methods[ix]);

			if(dlerror()) {
				return false;
			}

		}

		Logger::String{"GLib mainloop is available"}.trace();
		return false;
	}

	Glib::MainLoop::MainLoop() : Udjat::MainLoop(MainLoop::GLib) {

	}

	Glib::MainLoop::~MainLoop() {

	}

	int Glib::MainLoop::run() {
		return ENOTSUP;
	}

	bool Glib::MainLoop::active() const noexcept {
		return true;
	}

	void Glib::MainLoop::wakeup() noexcept {
	}

	void Glib::MainLoop::quit() {
	}

	void Glib::MainLoop::push_back(MainLoop::Timer *timer) {

	}

	void Glib::MainLoop::remove(MainLoop::Timer *timer) {

	}

	void Glib::MainLoop::push_back(MainLoop::Handler *handler) {

	}

	void Glib::MainLoop::remove(MainLoop::Handler *handler) {

	}

	bool Glib::MainLoop::enabled(const Timer *timer) const noexcept {

	}

	bool Glib::MainLoop::enabled(const Handler *handler) const noexcept {

	}

 }

