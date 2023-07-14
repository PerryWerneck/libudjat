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
 #include <udjat/tools/event.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/tools/intl.h>
 #include <udjat/agent/abstract.h>

 #ifdef _WIN32
	#include <udjat/win32/registry.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	SystemService * SystemService::instance = nullptr;

	SystemService & SystemService::getInstance() {
		if(instance) {
			return *instance;
		}
		throw std::system_error(EINVAL,std::system_category(),"System service is not active");
	}

	SystemService::SystemService() {
		if(instance) {
			throw std::system_error(EBUSY,std::system_category(),"System service already active");
		}
		instance = this;
		Logger::console(false);
	}

	SystemService::~SystemService() {
		if(instance == this) {
			instance = nullptr;
		}
#ifdef _WIN32
		try {

			Win32::Registry registry("service",true);
			registry.remove("status");
			registry.remove("status_time");

		} catch(...) {

			// Ignore errors.

		}

#endif // _WIN32
	}

	int SystemService::run(int argc, char **argv, const char *definitions) {
		return Application::run(argc,argv,definitions);
	}

	int SystemService::run(int argc, char **argv) {
		return Application::run(argc,argv);
	}

	int SystemService::deinit(const char *definitions) {
		Udjat::Event::remove(this);
		return Application::deinit(definitions);
	}

	void SystemService::setup(const char *pathname, bool startup) noexcept {

		try {

			Application::setup(pathname,startup);

		} catch(const std::exception &e) {

			status(e.what());

		}

	}

	void SystemService::root(std::shared_ptr<Abstract::Agent> agent) {

		debug("----------------> Adding systemservice listeners on agent ",agent->name());

#ifdef HAVE_SYSTEMD
		Logger::String{"Watching global status from '",agent->name(),"'"}.info("systemd");
#endif // HAVE_SYSTEMD

		class Listener : public Activatable {
		public:
			constexpr Listener() : Activatable("syssrvc") {
			}

			bool activated() const noexcept override {
				return false;
			}

			void activate(const std::function<bool(const char *key, std::string &value)> UDJAT_UNUSED(&expander)) override {
				activate();
			}

			void activate() {

				try {

					SystemService &service = SystemService::getInstance();

					try {

						auto agent = Abstract::Agent::root();
						auto state = agent->state();

						if(state->ready()) {
							service.status( _( "System is ready" ));
						} else {
							String message{state->summary()};
							if(message.strip().empty()) {
								service.status( _( "System is not ready" ) );
							} else {
								service.status(message.c_str());
							}
						}

					} catch(const std::exception &e) {

						service.status(e.what());

					}

				} catch(const std::exception &e) {

					cerr << "service\tCant update service state:" << e.what() << endl;

				}

			}


		};

		auto listener = std::make_shared<Listener>();

		agent->push_back(
				(Abstract::Agent::Event) (Abstract::Agent::Event::STARTED|Abstract::Agent::Event::STATE_CHANGED),
				listener
		);

		Application::root(agent);

		listener->activate();
	}


 }
