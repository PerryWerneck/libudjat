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
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <private/misc.h>
 #include <udjat/tools/logger.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	SystemService * SystemService::instance = nullptr;

	SystemService::SystemService(const char *d) : definitions(d) {

		if(instance) {
			throw runtime_error("Can't start more than one system service");
		}

		Application::init();

		instance = this;

	}

	SystemService::~SystemService() {
		instance = nullptr;
	}

	SystemService * SystemService::getInstance() {
		if(instance) {
			return instance;
		}
		return nullptr;
	}

	std::shared_ptr<Abstract::Agent> SystemService::RootFactory() const {
		return Udjat::RootAgentFactory();
	}

	int SystemService::run() noexcept {

		/// @brief WatchDog timer
#ifdef HAVE_SYSTEMD
		class WatchDog : public MainLoop::Timer {
		protected:
			void on_timer() override {
				if(instance) {
					sd_notifyf(0,"WATCHDOG=1\nSTATUS=%s",instance->state()->to_string().c_str());
				}
			}

		public:
			WatchDog() = default;
		};

		WatchDog watchdog;
#endif // HAVE_SYSTEMD

		Logger::redirect(mode == SERVICE_MODE_FOREGROUND ? true : false);

		try {

			init();

#ifdef HAVE_SYSTEMD
			{
				uint64_t watchdog_timer = 0;
				int status = sd_watchdog_enabled(0,&watchdog_timer);
				if(status < 0) {
					warning() << "Can't get SystemD watchdog status: " << strerror(-status) << endl;
				} else if(status == 0) {
#ifdef DEBUG
					watchdog.reset(120000L);
					watchdog.enable();
					info() << "SystemD watchdog set to " << watchdog.to_string() << endl;
#else
					warning() << "SystemD watchdog is not set" << endl;
#endif // DEBUG
				} else {
					watchdog.reset(watchdog_timer/2000L);
					watchdog.enable();
					info() << "SystemD watchdog set to " << watchdog.to_string() << endl;
				}

			}
#endif // HAVE_SYSTEMD

			MainLoop::getInstance().run();

			deinit();
			return 0;

		} catch(const std::exception &e) {

			error() << e.what() << endl;

		} catch(...) {

			error() << "Unexpected error" << endl;

		}

		return -1;
	}

	int SystemService::cmdline(int argc, const char **argv) {

		while(--argc > 0) {

			int rc = 0;
			const char *arg = *(++argv);

			try {

				if(!(strcmp(arg,"-h") && strcmp(arg,"--help") && strcmp(arg,"/h") && strcmp(arg,"/?") && strcmp(arg,"-?"))) {
					usage();
					cout << endl;
					mode = SERVICE_MODE_NONE;
					return 0;
				}

				if(!(strcmp(arg,"-f") && strcasecmp(arg,"--foreground") && strcmp(arg,"/f"))) {
					mode = SERVICE_MODE_FOREGROUND;
					continue;
				}

				if(!(strcmp(arg,"-d") && strcasecmp(arg,"--daemon") && strcmp(arg,"/d"))) {
					mode = SERVICE_MODE_DAEMON;
					continue;
				}

				if(arg[0] == '-' && arg[1] == '-') {

					// --param=value

					arg += 2;
					const char *ptr = strchr(arg,'=');
					if(ptr) {
						rc = cmdline(string(arg,ptr-arg).c_str(),ptr+1);
					} else {
						rc = cmdline(arg);
					}
				} else if(arg[0] == '-' && arg[1] && arg[2] == 0) {

					// -P value
					if(argc > 1 && argv[1] && argv[1][0] != '-') {
						rc = cmdline(arg[1], *(++argv));
						argc--;
					} else {
						rc = cmdline(arg[1]);
					}

				} else if(arg[0] == '/' && arg[1] && arg[2] == 0) {

					// /P value
					rc = cmdline(arg[1]);

				}

			} catch(const std::exception &e) {

				cerr << name() << "\t" << e.what() << endl;
				rc = -1;

			} catch(...) {

				cerr << name() << "\tUnexpected error" << endl;
				rc = -1;

			}

			if(rc) {
				return rc;
			}
		}

		return 0;
	}

	std::shared_ptr<Abstract::State> SystemService::state() const {

		auto agent = Abstract::Agent::root();

		if(agent) {
			return agent->state();
		}

		return make_shared<Abstract::State>("no-messages",Level::unimportant,"No messages","Service is running with no messages");

	}

	void SystemService::notify() noexcept {
		notify(state()->to_string().c_str());
	}


 }
