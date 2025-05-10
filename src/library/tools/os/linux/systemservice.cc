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

 /*
 static const struct {
	char to;
	const char *from;
	const char *help;
 } options[] = {
	{ 'd',	"daemon",	"\t\tRun in background" }
 };
 */

 namespace Udjat {

	int SystemService::setup(const char *definitions) {

		if(pop('d',"daemon")) {
			mode = Daemon;
		}

		if(pop('f',"foreground")) {
			mode = Foreground;
		}

		return Application::setup(definitions);
	}

	/// @brief Show help text to stream.
	/// @param out The stream for help.
	void SystemService::help(size_t width) const noexcept {

		Application::help(width);

		static const Application::Option values[] = {
			{ 'd', "daemon", _("Run in the background") },
			{ 'f', "foreground", _("Run in the foreground") },
		};
		
		for(const auto &value : values) {
			value.print(cout,20);
			cout << "\n";
		};

	}

	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

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

		return rc;
	}

	int SystemService::run(const char *definitions) {

		if(pop('h',"help")) {

			cout << _("Usage:") << "\n  " << argv[0]
				<< " " << _("[OPTION..]") << "\n\n";

			help();
			cout << "\n";
			
			Logger::help();

			return 0;
		}

		Logger::redirect();
#ifdef DEBUG 
		Logger::console(true);
#endif // DEBUG

		if(!MainLoop::getInstance()) {
			return -1;
		}

		int rc = 0;

		if(mode == Daemon) {
			Logger::console(false);
			if(daemon(0,0)) {
				int err = errno;
				Logger::String{"Error activating daemon mode: ",strerror(err)," (rc=",err,")"}.error("service");
				return err;
			}
		}

		if(mode != None) {

			try {

#ifdef HAVE_SYSTEMD
				class WatchDog : public MainLoop::Timer {
				protected:
					void on_timer() override {
						sd_notify(0,"WATCHDOG=1");
					}

				public:

					WatchDog() {

						uint64_t watchdog_timer = 0;
						int status = sd_watchdog_enabled(0,&watchdog_timer);

#ifdef DEBUG
						if(status == 0) {
							watchdog_timer = 120000000L;
							status = 1;
						}
#endif // DEBUG
						if(status < 0) {

							Logger::String{"Can't get SystemD watchdog status: ",strerror(-status)}.error("systemd");

						} else if(status == 0) {

							Logger::String{"SystemD watchdog is not set"}.warning("systemd");

						} else {

							reset(watchdog_timer/3000L);
							enable();
							Logger::String{"SystemD watchdog set to ",this->to_string()}.info("systemd");

						}

					}

				};

				WatchDog watchdog;

				sd_notifyf(0,"MAINPID=%lu",(unsigned long) getpid());
				sd_notifyf(0,"STATUS=Starting");

#endif // HAVE_SYSTEMD

				rc = Application::run(definitions);

			} catch(const std::exception &e) {

				error() << e.what() << endl;
				rc = -1;

			}
		}

		return rc;

	}

	int SystemService::install(const char *) {
		error() << "Not supported on linux, use systemd" << endl;
		return 0;
	}

	void SystemService::status(const char *status) noexcept {

#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"STATUS=%s",status);
		if(Logger::enabled(Logger::Trace)) {
			Logger::String{status}.write((Logger::Level) (Logger::Debug+1),"systemd");
		}
#else
		if(Logger::enabled(Logger::Trace)) {
			Logger::String{status}.write((Logger::Level) (Logger::Debug+1),Name().c_str());
		}
#endif // HAVE_SYSTEMD


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

