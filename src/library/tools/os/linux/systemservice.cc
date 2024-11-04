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
 #include <unistd.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 static const struct {
	char to;
	const char *from;
	const char *help;
 } options[] = {
	{ 'd',	"daemon",	"\t\tRun in background" }
 };

 namespace Udjat {

	bool SystemService::argument(const char *opt, const char *optarg) {

		for(auto &option : options) {
			if(!strcasecmp(opt,option.from)) {
				return argument(option.to,optarg);
			}
		}

		return Application::argument(opt,optarg);
	}

	/// @brief Set command-line argument.
	/// @param name argument name.
	/// @param value argument value.
	/// @return true if the argument was parsed.
	bool SystemService::argument(const char opt, const char *optarg) {

		switch(opt) {
		case 'f':
			mode = Foreground;
			break;

		case 'd':
		case 'D':
			mode = Daemon;
			return true;

		}

		return Application::argument(opt,optarg);
	}

	/// @brief Show help text to stdout.
	void SystemService::help(std::ostream &out) const noexcept {

		Application::help(out);

		for(auto &option : options) {
			out << "  --" << option.from << option.help << endl;
		}

	}

	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

		// TODO: Install unhandled exception manager.
		// https://en.cppreference.com/w/cpp/error/set_terminate

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

		if(!MainLoop::getInstance()) {
			return 0;
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

