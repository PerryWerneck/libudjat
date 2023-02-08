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
 #include <udjat/tools/configuration.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <private/misc.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/tools/string.h>
 #include <private/updater.h>

 #ifdef _WIN32
	#include <udjat/win32/registry.h>
 #endif // _WIN32

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	using Event = Abstract::Agent::Event;

	SystemService * SystemService::instance = nullptr;

	SystemService::SystemService(const char *d) : definitions{d} {

		debug("Creating system service");

		if(instance) {
			debug("Instance is not null");
			throw runtime_error("Can't start more than one system service");
		}

		/*
		if(!definitions) {

			// No definitions, try to detect.
			std::string options[] = {
#ifndef _WIN32
				string{ string{"/etc/"} + Application::name() + ".xml.d" },
#endif // _WIN32
				Application::DataFile{ (Application::name() + ".xml").c_str() },
				Application::DataFile{"xml.d"},
			};

			for(size_t ix=0;ix < (sizeof(options)/sizeof(options[0]));ix++) {

				debug("Searching for '",options[ix].c_str(),"' ",access(options[ix].c_str(), R_OK));

				if(access(options[ix].c_str(), R_OK) == 0) {
					debug("Detected service configuration in '",options[ix],"'");
					definitions = Quark(options[ix]).c_str();
					break;
				}

			}

			if(!definitions) {
				throw system_error(ENOENT,system_category(),string{"Cant find '"} + Application::DataFile{"xml.d"} + "'");
			}

		} else if(definitions[0] != '.' && definitions[0] != '/' && ::access(definitions,F_OK) != 0) {

			definitions = Quark(Application::DataFile{definitions}).c_str();

		}

		if(::access(definitions,R_OK) != 0) {
#ifdef _WIN32
			notify( (string{"Cant find '"} + definitions + "'").c_str() );
#endif //  _WIN32
			throw system_error(ENOENT,system_category(),definitions);
		}
		*/

		instance = this;

	}

	SystemService::~SystemService() {
		if(update_timer) {
			delete update_timer;
			update_timer = nullptr;
		}
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

	int SystemService::autostart(const char *) {
		return ENOTSUP;
	}

	int SystemService::shortcut(const char *) {
		return ENOTSUP;
	}

	int SystemService::run() noexcept {

		int rc = -1;

		/// @brief WatchDog timer
#ifdef HAVE_SYSTEMD
		class WatchDog : public MainLoop::Timer {
		protected:
			void on_timer() override {
				if(instance) {
					std::string state{instance->state()->to_string()};
					sd_notifyf(0,"WATCHDOG=1\nSTATUS=%s",state.c_str());
					if(Logger::enabled(Logger::Trace)) {
						Logger::String{state}.write((Logger::Level) (Logger::Debug+1),"SystemD");
					}
				} else {
					cerr << "SystemD\tNo System service instance while emitting systemd status" << endl;
				}
			}

		public:
			WatchDog() = default;
		};

		WatchDog watchdog;
#endif // HAVE_SYSTEMD

		{
			auto root = Abstract::Agent::root();

			if(root) {

				class Listener : public Activatable {
				public:
					constexpr Listener() : Activatable("syssrvc") {
					}

					bool activated() const noexcept override {
						return false;
					}

					void activate(const std::function<bool(const char *key, std::string &value)> UDJAT_UNUSED(&expander)) override {

						auto service = SystemService::getInstance();
						auto agent = Abstract::Agent::root();

						if(instance && agent) {

							auto state = agent->state();

							if(state->ready()) {
								service->notify( _( "System is ready" ));
							} else {
								String message{state->summary()};
								if(message.strip().empty()) {
									service->notify( _( "System is not ready" ) );
								} else {
									service->notify(message.c_str());
								}
							}
						}
					}
				};

				root->push_back( (Event) (Event::STARTED|Event::STATE_CHANGED), std::make_shared<Listener>() );
			}
		}

		try {

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

					error() << "Can't get SystemD watchdog status: " << strerror(-status) << endl;

				} else if(status == 0) {

					warning() << "SystemD watchdog is not set" << endl;

				} else {

					watchdog.reset(watchdog_timer/2000L);
					watchdog.enable();
					Logger::String{"SystemD watchdog set to ",watchdog.to_string()}.trace(name().c_str());

				}

			}
#endif // HAVE_SYSTEMD

			rc = MainLoop::getInstance().run();

		} catch(const std::exception &e) {

			error() << e.what() << endl;
			rc = -1;

		} catch(...) {

			error() << "Unexpected error" << endl;
			rc = -1;

		}

		return rc;
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
		String message{state()->to_string()};
		if(!message.empty()) {
			notify(state()->to_string().c_str());
		}
	}

	void SystemService::setup(bool force) {

		Updater updater{definitions,force};
		load(updater);

		if(updater.refresh()) {

			auto agent = RootFactory();

			updater.load(agent);

#if defined(HAVE_SYSTEMD)

			sd_notifyf(0,"READY=1\nSTATUS=%s",agent->state()->to_string().c_str());

#elif defined(_WIN32)

			try {

				Win32::Registry registry("service",true);

				registry.set("status",agent->state()->to_string().c_str());
				registry.set("status_time",TimeStamp().to_string().c_str());

			} catch(const std::exception &e) {

				error() << "Cant update windows registry: " << e.what() << endl;

			}

#endif

		} else {

			Logger::String{"Keeping the actual root agent"}.write((Logger::Level) (Logger::Debug+1),Application::Name().c_str());
#ifdef HAVE_SYSTEMD
			sd_notifyf(0,"READY=1\nSTATUS=%s",Abstract::Agent::root()->state()->to_string().c_str());
#endif // HAVE_SYSTEMD

		}

		debug("------------------------------------------------------------------------------");

		{
			time_t wait = updater.wait();

			if(!wait) {

				Logger::String{"Auto update is disabled"}.write((Logger::Level) (Logger::Debug+1),Application::Name().c_str());

			} else {

				update_timer = MainLoop::getInstance().TimerFactory(wait * 1000,[this](){

					ThreadPool::getInstance().push([this](){

						Logger::String{"Starting auto update"}.write((Logger::Level) (Logger::Debug+1),Application::Name().c_str());
#ifdef HAVE_SYSTEMD
						sd_notifyf(0,"STATUS=%s","Running auto update");
#endif // HAVE_SYSTEMD

						setup(false);

					});

					update_timer = nullptr;
					return false;
				});

				Logger::String{"Next auto update set to ",TimeStamp{time(0)+wait}.to_string()}.write((Logger::Level) (Logger::Debug+1),Application::Name().c_str());

			}

		}


	}

 }
