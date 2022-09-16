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
 #include <udjat/tools/event.h>
 #include <udjat/tools/application.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	void SystemService::notify(const char *message) noexcept {
#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"STATUS=%s",message);
#endif // HAVE_SYSTEMD
		info() << message << endl;
	}

	void SystemService::init() {

		string appconfig;
		if(!definitions) {
			Config::Value<string> config("service","definitions","");
			if(config.empty()) {
				definitions = Quark(Application::DataDir("xml.d")).c_str();
			} else {
				definitions = Quark(config).c_str();
			}
		}

		if(!Module::preload(definitions)) {
			throw runtime_error("Module preload has failed, aborting service");
		}

		if(definitions[0] && strcasecmp(definitions,"none")) {

			info() << "Loading service definitions from " << definitions << endl;

			reconfigure(definitions,true);

			Config::Value<string> signame("service","signal-reconfigure","SIGHUP");
			if(!signame.empty() && strcasecmp(signame.c_str(),"none")) {
				Udjat::Event &reconfig = Udjat::Event::SignalHandler(this,signame.c_str(),[this](){
					reconfigure(definitions,false);
					return true;
				});
				info() << signame << " (" << reconfig.to_string() << ") triggers a conditional reload" << endl;
			}

		}

	}

	void SystemService::deinit() {
		Udjat::Event::remove(this);
	}

	void SystemService::stop() {
		MainLoop::getInstance().quit();
	}

	int SystemService::run() {

#ifdef HAVE_SYSTEMD
		uint64_t watchdog_timer = 0;

		{

			int status = sd_watchdog_enabled(0,&watchdog_timer);

			if(status < 0) {

				cout << "systemd\tError '" << strerror(-status) << "' getting watchdog status" << endl;

			} else if(status == 0) {

				cout << "systemd\tWatchdog timer is not set" << endl;

			} else {

				// SystemD watchdog is enabled.
				if(watchdog_timer > 0) {

					cout << "systemd\tWatchdog timer is set to " << (watchdog_timer / 1000000L) << " seconds" << endl;

					MainLoop::getInstance().TimerFactory(&watchdog_timer,(unsigned long) (watchdog_timer / 2000L),[this](){
						sd_notifyf(0,"WATCHDOG=1\nSTATUS=%s",state()->to_string().c_str());
						return true;
					});

				} else {

					cout << "systemd\tWatchdog timer is disabled" << endl;

				}
			}

		}

#endif // HAVE_SYSTEMD

		MainLoop::getInstance().run();

#ifdef HAVE_SYSTEMD
		MainLoop::getInstance().remove(&watchdog_timer);
#endif // HAVE_SYSTEMD

		return 0;
	}

	void SystemService::usage() const noexcept {
		cout 	<< "Usage: " << endl << "  " << name() << " [options]" << endl << endl
				<< "  --core\tenable coredumps" << endl
				<< "  --daemon\tRun " << name() << " service in the background" << endl
				<< "  --foreground\tRun " << name() << " service as application (foreground)" << endl;
	}

	int SystemService::cmdline(char key, const char UDJAT_UNUSED(*value)) {

		switch(key) {
		case 'f':	// Run in foreground.
			{
				cout << "Starting " << name () << " application" << endl << endl;
				Logger::redirect(true);
				init();
				try {
					run();
				} catch(const std::exception &e) {
					error() << "Error '" << e.what() << "' running service" << endl;
				} catch(...) {
					error() << "Unexpected error running service" << endl;
				}

				deinit();
				return 0;
			}
			break;

		case 'd':	// Run as a daemon.
			{

				if(daemon(0,0)) {
					throw std::system_error(errno, std::system_category());
				}

				Logger::redirect();
				init();
				run();
				deinit();
				return 0;

			}
			break;

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

		// The default options doesn't have values, then, reject here.
		if(value) {
			return ENOENT;
		}

		static const struct {
			char option;
			const char *key;
		} options[] = {
			{ 'C', "core" },
			{ 'd', "daemon" },
			{ 'f', "foreground" }
		};

		for(size_t option = 0; option < (sizeof(options)/sizeof(options[0])); option++) {
			if(!strcasecmp(key,options[option].key)) {
				return cmdline(options[option].option);
			}
		}

		return ENOENT;
	}

	int SystemService::run(int argc, char **argv) {

		if(argc > 1) {
			int rc = cmdline(argc,(const char **) argv);
			if(rc != -2) {
				return rc;
			}
		}

		// Run as service by default.
		try {

			cout << "Starting " << name() << " service" << endl;
			Logger::redirect();
			init();
			int rc = run();
			deinit();
			return rc;

		} catch(const std::exception &e) {
			error() << e.what() << endl;
		} catch(...) {
			error() << "\tUnexpected error" << endl;
		}

		return -1;

	}

 }

