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
		METHOD_G_MAIN_LOOP_NEW,
		METHOD_G_MAIN_CONTEXT_DEFAULT,
		METHOD_G_MAIN_LOOP_RUN,
		METHOD_G_MAIN_LOOP_UNREF
	};

	static const char *names[] = {
		"g_timeout_add_full",
		"g_source_remove",
		"g_io_channel_unix_new",			// https://docs.gtk.org/glib/ctor.IOChannel.unix_new.html
		"g_io_channel_unref",				// https://docs.gtk.org/glib/method.IOChannel.unref.html
		"g_io_channel_set_encoding",
		"g_io_add_watch",
		"g_idle_add_once",
		"g_main_loop_new",
		"g_main_context_default",
		"g_main_loop_run",
		"g_main_loop_unref"
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

		void * (*g_main_loop_new)(void * context, int is_running) 
			= (void * (*)(void *, int)) methods[METHOD_G_MAIN_LOOP_NEW];	

		void * (*g_main_context_default)(void) 
			= (void * (*)(void)) methods[METHOD_G_MAIN_CONTEXT_DEFAULT];

		void (*g_main_loop_run)(void* loop)
			= (void (*)(void *)) methods[METHOD_G_MAIN_LOOP_RUN];

		void (*g_main_loop_unref)(void* loop)
			= (void (*)(void *)) methods[METHOD_G_MAIN_LOOP_UNREF];

		void *loop = g_main_loop_new(g_main_context_default(), 0);
		g_main_loop_run(loop);
		g_main_loop_unref(loop);
	
		return 0;
	
	}

	bool Glib::MainLoop::active() const noexcept {
		return true;
	}

	void Glib::MainLoop::post(MainLoop::Message *message) noexcept {
		unsigned int (*g_idle_add_once)(void *function, void * data) =
			(unsigned int (*)(void *function, void * data)) methods[METHOD_G_IDLE_ADD_ONCE];
		g_idle_add_once((void *) MainLoop::Message::on_posted, (void *) message);
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
