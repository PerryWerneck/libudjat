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

 /*
 #include <private/misc.h>
 #include <udjat/tools/systemservice.h>
 #include <iostream>
 #include <system_error>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
 #include <unistd.h>
 #include <fstream>
 #include <sys/time.h>
 #include <sys/resource.h>
 #include <locale.h>
 #include <udjat/agent.h>
 #include <csignal>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/application.h>
 */

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	int SystemService::argument(char opt, const char *optstring) {

		switch(opt) {
		case 'S':
		case 'Q':
			cerr << "Not supported on linux, use systemd" << endl;
			return ENOTSUP;

		case 'f':
			mode = Foreground;
			return Application::argument(opt,optstring);

		case 'D':
			mode = Daemon;
			break;

		default:
			return Application::argument(opt,optstring);

		}

		return 1;
	}

	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

		int rc = Application::init(definitions);

		if(!rc) {
			Config::Value<string> signame("service","signal-reconfigure","SIGHUP");
			if(!signame.empty() && strcasecmp(signame.c_str(),"none")) {

				Udjat::Event &reconfig = Udjat::Event::SignalHandler(this,signame.c_str(),[this,definitions](){
					ThreadPool::getInstance().push([this,definitions](){
						setup(definitions);
					});
					return true;
				});
				info() << signame << " (" << reconfig.to_string() << ") triggers a conditional reload" << endl;
			}
		}

		return rc;
	}

	int SystemService::run(const char *definitions) {

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

						std::string state;

						auto agent = Abstract::Agent::root();
						if(agent) {
							state = agent->state()->to_string();
						} else {
							state = _("Agent subsystems is not running");
						}

						sd_notifyf(0,"WATCHDOG=1\nSTATUS=%s",state.c_str());

						if(Logger::enabled(Logger::Info)) {
							Logger::String{state}.write((Logger::Level) (Logger::Debug+1),"system");
						}

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


 /*
	void SystemService::notify(const char *message) noexcept {

		if(message && *message) {

#ifdef HAVE_SYSTEMD
			sd_notifyf(0,"STATUS=%s",message);
#endif // HAVE_SYSTEMD

			Logger::write((Logger::Level) (Logger::Debug+1),name().c_str(),message);

		}

	}

	void SystemService::init() {

		// TODO: Install unhandled exception manager.
		// https://en.cppreference.com/w/cpp/error/set_terminate
		setup(true);

		Config::Value<string> signame("service","signal-reconfigure","SIGHUP");
		if(!signame.empty() && strcasecmp(signame.c_str(),"none")) {
			Udjat::Event &reconfig = Udjat::Event::SignalHandler(this,signame.c_str(),[this](){
				ThreadPool::getInstance().push([this](){
					setup(false);
				});
				return true;
			});
			info() << signame << " (" << reconfig.to_string() << ") triggers a conditional reload" << endl;
		}

	}

	void SystemService::deinit() {
		Udjat::Event::remove(this);
	}

	int SystemService::install() {
		return ENOTSUP;
	}

	void SystemService::stop() {
		MainLoop::getInstance().quit();
	}

	void SystemService::usage() const noexcept {
		cout 	<< "Usage: " << endl << "  " << name() << " [options]" << endl << endl
				<< "  --core\t\tenable coredumps" << endl
				<< "  --timer=seconds\tTerminate " << name() << " after 'seconds'" << endl
				<< "  --daemon\t\tRun " << name() << " service in the background" << endl
				<< "  --foreground\t\tRun " << name() << " service as application (foreground)" << endl;
	}

	int SystemService::cmdline(char key, const char *value) {

		switch(key) {
		case 'i':	// Install
			return install();

		case 'T':	// Auto quit
			{
				if(!value) {
					throw system_error(EINVAL,system_category(),_( "Invalid timer value" ));
				}

				int seconds = atoi(value);
				if(!seconds) {
					throw system_error(EINVAL,system_category(),_( "Invalid timer value" ));
				}

				MainLoop::getInstance().TimerFactory(seconds * 1000,[](){
					Application::warning() << "Exiting by timer request" << endl;
					MainLoop::getInstance().quit();
					return false;
				});

			}
			return 0;

		case 'C':	// Enable core dumps.
			{
				if(optarg && *optarg) {
					ofstream ofs;
					ofs.open("/proc/sys/kernel/core_pattern",ofstream::out);
					ofs << optarg;
					ofs.close();
				}

				// Enable cores
				struct rlimit core_limits;
				core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;

				if(setrlimit(RLIMIT_CORE, &core_limits)) {
					error() << "Error \"" << strerror(errno) << "\" activating coredumps" << endl;
				} else {
					info() << "Coredumps are active" << endl;
				}
			}
			return -2;

		}

		return ENOENT;
	}

	int SystemService::cmdline(const char *key, const char *value) {

		static const struct {
			char option;
			const char *key;
		} options[] = {
			{ 'i', "install" },
			{ 'C', "core" },
			{ 'd', "daemon" },
			{ 'f', "foreground" },
			{ 'T', "timer" }
		};

		for(size_t option = 0; option < (sizeof(options)/sizeof(options[0])); option++) {
			if(!strcasecmp(key,options[option].key)) {
				return cmdline(options[option].option,value);
			}
		}

		return ENOENT;
	}

	int SystemService::run(int argc, char **argv) {

		int rc = 0;

		if(argc > 1) {
			rc = cmdline(argc,(const char **) argv);
			if(rc) {
				mode = SERVICE_MODE_NONE;
			}
		}

		Logger::redirect(mode == SERVICE_MODE_FOREGROUND ? true : false);

		if(mode == SERVICE_MODE_DAEMON) {
			if(daemon(0,0)) {
				error() << strerror(errno) << endl;
				return -1;
			}
		}

		if(mode != SERVICE_MODE_NONE) {

			try {

				init();
				rc = run();
				deinit();

			} catch(const std::exception &e) {

				error() << e.what() << endl;
				rc = -1;

			}
		}

		return rc;

	}

	void SystemService::load(std::list<std::string> &files) {
	}

	int SystemService::wakeup() {
		return ENOTSUP;
	}

*/

 }

