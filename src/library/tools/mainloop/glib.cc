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

#ifdef DEBUG 

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>
 #include <private/glib/mainloop.h>
 #include <udjat/tools/timer.h>
 #include <dlfcn.h>

 namespace Udjat {

	enum methods {
		METHOD_G_TIMEOUT_ADD_FULL,
		METHOD_G_SOURCE_REMOVE,
		METHOD_G_IO_CHANNEL_UNIX_NEW,
		METHOD_G_IO_CHANNEL_UNREF,
		METHOD_G_IO_CHANNEL_SET_ENCODING,
		METHOD_G_IO_ADD_WATCH,
		METHOD_G_IDLE_ADD_ONCE,
	};

	static const char *names[] = {
		"g_timeout_add_full",
		"g_source_remove",
		"g_io_channel_unix_new",			// https://docs.gtk.org/glib/ctor.IOChannel.unix_new.html
		"g_io_channel_unref",				// https://docs.gtk.org/glib/method.IOChannel.unref.html
		"g_io_channel_set_encoding",
		"g_io_add_watch",
		"g_idle_add_once",
	};

	static void *methods[sizeof(names)/sizeof(names[0])];

	bool Glib::MainLoop::available() noexcept {

		for(size_t ix = 0; ix < sizeof(names)/sizeof(names[0]); ix++) {

			dlerror();
			methods[ix] = dlsym(RTLD_DEFAULT, names[ix]);

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

	struct bgcal {
		std::function<void(const void *)> call;
		char buffer[0];
	};

	static void do_bgcall(struct bgcal *data) {
		data->call(data->buffer);
		free(data);
	}

	void Glib::MainLoop::post(void *msg, size_t msglen, const std::function<void(const void *)> &call) {

		unsigned int (*g_idle_add_once)(void *function, void * data) =
			(unsigned int (*)(void *function, void * data)) methods[METHOD_G_IDLE_ADD_ONCE];
		
		struct bgcal *bgcal = (struct bgcal *) malloc(sizeof(struct bgcal) + msglen + 1);
		bgcal->call = call;
		memcpy(bgcal->buffer,msg,msglen);

		g_idle_add_once((void *) do_bgcall, (void *) bgcal);

	}

	void Glib::MainLoop::wakeup() noexcept {
	}

	void Glib::MainLoop::quit() {
	}

	static int do_timer(Udjat::MainLoop::Timer *timer) {

		if(!timer->activate()) {
			return 0;
		}

		return -1;
	}

	void Glib::MainLoop::on_timer_removed(Udjat::MainLoop::Timer *timer) {
		Glib::MainLoop *mainloop = dynamic_cast<Glib::MainLoop *>(&Udjat::MainLoop::getInstance());
		if(mainloop) {
			mainloop->timers.remove_if([timer](const Timer &t) {
				return t.timer == timer;
			});
		}
	}

	void Glib::MainLoop::push_back(Udjat::MainLoop::Timer *timer) {

		unsigned int (*g_timeout_add_full)(int priority, unsigned int interval, void *function, void *data, void *notify)
			= (unsigned int (*)(int, unsigned int, void *, void *, void *)) methods[METHOD_G_TIMEOUT_ADD_FULL];

		unsigned int id = 
			g_timeout_add_full(
				0, // G_PRIORITY_DEFAULT, 
				timer->interval(), 
				(void *) do_timer, 
				(void *) timer, 
				(void *) on_timer_removed
		);

		timers.emplace_back(id,timer);

	}

	void Glib::MainLoop::remove(Udjat::MainLoop::Timer *tm) {

		int (*g_source_remove)(unsigned int id) = (int (*)(unsigned int)) methods[METHOD_G_SOURCE_REMOVE];

		for(auto &timer : timers) {
			if(timer.timer == tm) {
				g_source_remove(timer.id);
				break;
			}
		}

	}

	void Glib::MainLoop::push_back(Udjat::MainLoop::Handler *handler) {

	}

	void Glib::MainLoop::remove(Udjat::MainLoop::Handler *handler) {

	}

	bool Glib::MainLoop::enabled(const Udjat::MainLoop::Timer *tm) const noexcept {
		for(auto &timer : timers) {
			if(timer.timer == tm) {
				return true;
			}
		}
		return false;
	}

	bool Glib::MainLoop::enabled(const Udjat::MainLoop::Handler *handler) const noexcept {

		return false;
	}

 }

#endif // DEBUG
