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
 #include <udjat/tools/systemservice.h>
 #include <iostream>
 #include <system_error>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <getopt.h>
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

	SystemService * SystemService::instance = nullptr;

	void SystemService::onReloadSignal(int UDJAT_UNUSED(signal)) noexcept {

		cout << "service\tReconfigure request received from signal SIGUSR1" << endl;

		try {

			ThreadPool::getInstance().push([](){
				if(instance && instance->definitions) {

					Application::DataFile path(instance->definitions);
					cout << Application::Name() << "\tReconfiguring from '" << path << "'" << endl;
					Udjat::load(path.c_str());

				} else {

					clog << "service\tUnable to handle SIGUSR1, no service or no defined file(s)" << endl;

				}
			});

		} catch(const std::exception &e) {

			cerr << "service\t" << e.what() << endl;

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

			// Inicialize from XML
			Application::DataFile path(definitions);
			cout << Application::Name() << "\tInitializing from '" << path << "'" << endl;
			Udjat::load(path.c_str());

			// Capture reload signal.
			if(!instance) {
				instance = this;
				signal(SIGUSR1,onReloadSignal);
				cout << Application::Name() << "\tUse SIGUSR1 to force reload of " << path << endl;
			}

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
				<< "  --foreground\tRun " << appname << " service as application (foreground)" << endl
				<< endl;
	}

	int SystemService::run(int argc, char **argv) {

		auto appname = Application::Name::getInstance();
		int rc = 0;

		// Parse command line options.
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
			static struct option options[] = {
				{ "foreground",		no_argument,		0,	'f' },
				{ "daemon",			no_argument,		0,	'd' },
				{ "core",			optional_argument,	0,	'C' },
				{ "help",			no_argument,		0,	'h' },
				{ NULL }
			};
			#pragma GCC diagnostic pop

			int long_index =0;
			int opt;
			while((opt = getopt_long(argc, argv, "fdC:h", options, &long_index )) != -1) {
				switch(opt) {
				case 'h':
					usage(appname.c_str());
					return 0;

				case 'f':	// Run in foreground.
					try {

						Logger::redirect(nullptr,true);
						cout << appname << "\tStarting in application mode" << endl;
						init();
						rc = run();
						deinit();
						return rc;

					} catch(const std::exception &e) {
						cerr << appname << "\t" << e.what() << endl;
						return -1;
					} catch(...) {
						cerr << appname << "\tUnexpected error" << endl;
						return -1;
					}
					break;

				case 'd':	// Run as a daemon.
					try {

						if(daemon(0,0)) {
							throw std::system_error(errno, std::system_category());
						}

						Logger::redirect();
						init();
						run();
						deinit();
						return 0;

					} catch(const std::exception &e) {
						cerr << appname << "\t" << e.what() << endl;
						return -1;
					} catch(...) {
						cerr << appname << "\tUnexpected error" << endl;
						return -1;
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
					break;

				default:
					return -1;

				}

			}

		}

		// Run as service by default.
		try {

			cout << "Running " << appname << " service" << endl;
			Logger::redirect();
			init();
			rc = run();
			deinit();

		} catch(const std::exception &e) {
			cerr << appname << "\t" << e.what() << endl;
			return -1;
		} catch(...) {
			cerr << appname << "\tUnexpected error" << endl;
			return -1;
		}

		return rc;

	}

 }

