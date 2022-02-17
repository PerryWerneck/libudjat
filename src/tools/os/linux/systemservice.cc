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
 #include <udjat-internals.h>
 #include <udjat/tools/systemservice.h>
 #include <iostream>
 #include <system_error>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>
 #include <fstream>
 #include <sys/time.h>
 #include <sys/resource.h>
 #include <locale.h>
 #include <udjat/agent.h>
 #include <csignal>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {

	void SystemService::onReloadSignal(int signal) noexcept {

		Application::info() << "Reconfigure request received from signal " << strsignal(signal) << endl;

		try {

			ThreadPool::getInstance().push([](){
				if(instance) {
					instance->reconfigure(instance->definitions);
				}
			});

		} catch(const std::exception &e) {

			Application::error() << e.what() << endl;

		}

	}

	SystemService::~SystemService() {
		if(instance == this) {
			deinit();
			instance = nullptr;
		}
	}

	void SystemService::init() {
		setlocale( LC_ALL, "" );

		if(definitions) {
			reconfigure(definitions);
		}

	}

	void SystemService::deinit() {
		if(instance == this) {
			signal(SIGUSR1,SIG_DFL);
			instance = nullptr;
		}
	}

	void SystemService::stop() {
		MainLoop::getInstance().quit();
	}

	int SystemService::run() {
		MainLoop::getInstance().run();
		return 0;
	}

	void SystemService::usage(const char *appname) const noexcept {
		cout 	<< "Usage: " << endl << "  " << appname << " [options]" << endl << endl
				<< "  --core\tenable coredumps" << endl
				<< "  --daemon\tRun " << appname << " service in the background" << endl
				<< "  --foreground\tRun " << appname << " service as application (foreground)" << endl;
	}

	int SystemService::cmdline(const char *appname, char key, const char UDJAT_UNUSED(*value)) {

		switch(key) {
		case 'f':	// Run in foreground.
			{
				Logger::redirect(true);
				cout << appname << "\tStarting in application mode" << endl;
				init();
				run();
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
				if(optarg) {
					ofstream ofs;
					ofs.open("/proc/sys/kernel/core_pattern",ofstream::out);
					ofs << optarg;
					ofs.close();
				}

				// Enable cores
				struct rlimit core_limits;
				core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;

				if(setrlimit(RLIMIT_CORE, &core_limits)) {
					cerr << appname << "\tError \"" << strerror(errno) << "\" activating coredumps" << endl;
				} else {
					cout << appname << "\tCoredumps are active" << endl;
				}
			}
			return -2;

		}

		return ENOENT;
	}

	int SystemService::cmdline(const char *appname, const char *key, const char *value) {

		// The default options doesn't have values, then, reject here.
		if(value) {
			return ENOENT;
		}

		static const struct {
			char option;
			const char *key;
		} options[] = {
			{ 'C', "coredump" },
			{ 'd', "daemon" },
			{ 'f', "foreground" }
		};

		for(size_t option = 0; option < (sizeof(options)/sizeof(options[0])); option++) {
			if(!strcasecmp(key,options[option].key)) {
				return cmdline(appname, options[option].option);
			}
		}

		return ENOENT;
	}

	int SystemService::run(int argc, char **argv) {

		auto appname = Application::Name::getInstance();

		if(argc > 1) {
			int rc = cmdline(appname.c_str(),argc,(const char **) argv);
			if(rc != -2) {
				return rc;
			}
		}

		// Run as service by default.
		try {

			cout << "Running " << appname << " service" << endl;
			Logger::redirect();
			init();
			int rc = run();
			deinit();
			return rc;

		} catch(const std::exception &e) {
			cerr << appname << "\t" << e.what() << endl;
		} catch(...) {
			cerr << appname << "\tUnexpected error" << endl;
		}

		return -1;

	}

 }

