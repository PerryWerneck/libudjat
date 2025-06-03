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
 #include <udjat/tools/application.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/agent/abstract.h>
 #include <stdexcept>
 #include <udjat/tools/event.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/console.h>
 #include <unistd.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	int SystemService::setup(const char *definitions) {

		return Application::setup(definitions);
	}

	/// @brief Show help text to stream.
	/// @param out The stream for help.
	void SystemService::help(size_t width) const noexcept {

		Application::help(width);

		static const CommandLineParser::Argument values[] = {
			{ 'd', "daemon", _("Run in the background") },
		};
		
		for(const auto &value : values) {
			value.print(cout,20);
			cout << "\n";
		};

	}

	int SystemService::run(const char *definitions) {

		if(has_argument('d',"daemon")) {
			if(daemon(0,0)) {
				int err = errno;
				Logger::String{"Error activating daemon mode: ",strerror(err)," (rc=",err,")"}.error("service");
				return err;
			}
			Logger::console(false);
		}

		return Application::run(definitions);

	}

	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"STATUS=Starting");
#endif // HAVE_SYSTEMD

#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"MAINPID=%lu",(unsigned long) getpid());
#endif // HAVE_SYSTEMD

		// Install unhandled exception manager.
		// https://en.cppreference.com/w/cpp/error/set_terminate
		std::set_terminate([]() {
#ifdef HAVE_SYSTEMD
			sd_notify(0,"STATUS=Unhandled exception");
			sd_notify(0,"STOPPING=1");
#endif // HAVE_SYSTEMD
			Logger::String{"Unhandled exception"}.error(PACKAGE_NAME);
			std::abort();
		});

		int rc = Application::init(definitions);
		if(rc) {
			debug("Application init rc was ",rc);
			return rc;
		}

		Config::Value<string> signame("service","signal-reconfigure","SIGHUP");
		if(!signame.empty() && strcasecmp(signame.c_str(),"none")) {

			Udjat::Event &reconfig = Udjat::Event::SignalHandler(this,signame.c_str(),[this,definitions](){
				ThreadPool::getInstance().push([this,definitions](){
					setup(definitions,false);
				});
				return true;
			});
			info() << signame << " (" << reconfig.to_string() << ") triggers a conditional reload" << endl;
		}

#ifdef HAVE_SYSTEMD
		{
			uint64_t watchdog_timer = 0;
			int status = sd_watchdog_enabled(0,&watchdog_timer);

#ifdef DEBUG
			if(status == 0) {
				watchdog_timer = 120000000L;
				status = 1;
			}
#endif // DEBUG

			if(status < 0) {

				Logger::String{"Can't get watchdog status: ",strerror(-status)}.error("systemd");

			} else if(status == 0) {

				Logger::String{"Watchdog is not set"}.warning("systemd");

			} else {

				reset(watchdog_timer/3000L);
				MainLoop::Timer::enable();
				Logger::String{"Watchdog set to ",MainLoop::Timer::to_string()}.write((Logger::Level) (Logger::Debug+1),"systemd");

			}

		}
#endif // HAVE_SYSTEMD
		
		return rc;
	}

	void SystemService::on_timer() {

		debug("Watchdog timer expired");
		
#ifdef HAVE_SYSTEMD
		// Notify systemd that we are alive.
		// https://www.freedesktop.org/software
		sd_notify(0,"WATCHDOG=1");
#endif // HAVE_SYSTEMD

	}

	Dialog::Status & SystemService::state(const Level level, const char *message) noexcept {
#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"STATUS=%s",message);
#endif // HAVE_SYSTEMD
		return Application::state(level,message);
	}

	int SystemService::install(const char *) {
		error() << "Not supported on linux, use systemd" << endl;
		return 0;
	}

	int SystemService::uninstall() {
		error() << "Not supported on linux, use systemd" << endl;
		return 0;
	}

	int SystemService::start() {
		error() << "Not supported on linux, use systemd" << endl;
		return 0;
	}

	int SystemService::stop() {
		error() << "Not supported on linux, use systemd" << endl;
		return 0;
	}


 }

