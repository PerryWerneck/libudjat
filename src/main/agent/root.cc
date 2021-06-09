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

 #include "private.h"
 #include <sys/utsname.h>
 #include <udjat/factory.h>
 #include <udjat/tools/sysconfig.h>
 #include <udjat/tools/virtualmachine.h>
 #include <cstring>
 #include <unistd.h>

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> getDefaultRootAgent() {

		class Agent : public Abstract::Agent {
		public:
			Agent(const char *name) : Abstract::Agent(name) {

				info("root agent was {}","created");
				this->icon = "computer";
				this->uri = Quark(string{"http://"} + name).c_str();

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

				//
				// Detect virtualization.
				//
				VirtualMachine vm;

#ifdef DEBUG
				cout << "Detected Virtual machine was '" << vm << "'" << endl;
#endif // DEBUG

				//
				// Get machine information
				//
				if(!vm) {

					try {

						static const char *ids[] = {
							"dmi.1.0.1",
							"dmi.1.0.2"
						};

						string id;

						for(size_t ix = 0; ix < (sizeof(ids)/sizeof(ids[0])); ix++) {

							if(!id.empty()) {
								id += " ";
							}

							id += Factory::get(ids[ix])->to_string();

						}

						if(!id.empty()) {
							this->summary = Quark(id).c_str();
						}

					} catch(const std::exception &e) {

						warning("{}",e.what());

					}
				} else {

					this->summary = Quark(Logger::Message("{} virtual machine",vm.to_string())).c_str();

				}

			}

			virtual ~Agent() {
				info("root agent was {}","destroyed");
			}

			void get(const Request &request, Response &response) override {

				Abstract::Agent::get(request,response);

				struct utsname uts;

				if(uname(&uts) >= 0) {
					response["system"] = string(uts.sysname) + " " + uts.release + " " + uts.version;
				}

			}

			bool activate(std::shared_ptr<Abstract::State> state) noexcept override {

				// TODO: Replace state if level <= Udjat::ready

				bool changed = Abstract::Agent::activate(state);

				// Update icon based on state.
				this->icon = (this->getState()->getLevel() <= Udjat::warning ? "computer" : "computer-fail");

				return changed;
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
