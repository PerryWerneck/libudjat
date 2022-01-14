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
 #include "private.h"

 #ifndef _WIN32
	#include <sys/utsname.h>
 #endif // _WIN32

 #include <udjat/factory.h>
 #include <udjat/tools/sysconfig.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 #include <cstring>
 #include <unistd.h>

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> getDefaultRootAgent() {

		class Agent : public Abstract::Agent {
		public:
			Agent(const char *name) : Abstract::Agent(name) {

				cout << "agent\tRoot node was created" << endl;
				this->icon = "computer";
				this->uri = Quark(string{"http://"} + name).c_str();

#ifndef _WIN32
				//
				// Get UNAME
				//
				struct utsname uts;

				if(uname(&uts) < 0) {
					memset(&uts,0,sizeof(uts));
					clog << getName() << "\tError '" << strerror(errno) << "' getting uts info" << endl;
				}

				//
				// Get OS information
				//
				try {

					SysConfig::File osrelease("/etc/os-release","=");

					string label = osrelease["PRETTY_NAME"].value;
					if(uts.machine[0]) {
						label += " ";
						label += uts.machine;
					}

					this->label = Quark(label).c_str();

				} catch(const std::exception &e) {

					error("Error '{}' reading system release file",e.what());

				}
#endif // _WIN32

#ifdef HAVE_VMDETECT
				{
					//
					// Detect virtualization.
					//
					VirtualMachine vm;

					//
					// Get machine information
					//
					if(!vm) {

						try {

							auto sysid = URL(Config::Value<string>("system","url-summary","dmi:///system/sku").c_str()).get();

							if(sysid->isValid()) {
								this->summary = Quark(sysid->c_str()).c_str();
							}

						} catch(const std::exception &e) {

							warning("{}",e.what());

						}
					} else {

						this->summary = Quark(Logger::Message("{} virtual machine",vm.to_string())).c_str();

					}
				}
#endif // HAVE_VMDETECT

			}

			std::shared_ptr<Abstract::State> stateFromValue() const override {

				class ReadyState : public Abstract::State {
				public:
					ReadyState() : Abstract::State("ready", ready, "System is ready", "No abnormal state was detected") {
					}

				};

				return make_shared<ReadyState>();

			}

			virtual ~Agent() {
				cout << "agent\tRoot node was destroyed" << endl;
			}

			void get(const Request UDJAT_UNUSED(&request), Response &response) override {

				getDetails(response);

#ifndef _WIN32
				struct utsname uts;

				if(uname(&uts) >= 0) {
					response["system"] = string(uts.sysname) + " " + uts.release + " " + uts.version;
				}
#endif // _WIN32

			}

			bool activate(std::shared_ptr<Abstract::State> state) noexcept override {

				if(!state->isReady()) {

					// The requested state is not ready, activate it.
					this->icon = "computer-fail";
					bool changed = Abstract::Agent::activate(state);
					return changed;

				}

				if(this->getState()->isReady()) {
					// Already ok, do not change.
					return false;
				}

				this->icon = "computer";
				super::activate(stateFromValue());

				return true;
			}

		};

		char hostname[255];
		if(gethostname(hostname, 255)) {
			cerr << "Error '" << strerror(errno) << "' getting hostname" << endl;
			strncpy(hostname,PACKAGE_NAME,255);
		}

		return make_shared<Agent>(Quark(hostname).c_str());

	}

 }
