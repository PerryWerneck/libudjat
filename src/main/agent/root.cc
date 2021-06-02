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
 #include <udjat/factory.h>
 #include <udjat/tools/sysconfig.h>

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> getDefaultRootAgent() {

		class Agent : public Abstract::Agent {
		public:
			Agent(const char *name) : Abstract::Agent(name) {

				info("root agent was {}","created");
				this->icon = "computer";
				this->uri = Quark(string{"http://"} + name).c_str();

				//
				// Get OS information
				//
				try {

					SysConfig::File osrelease("/etc/os-release","=");

					this->label = Quark(osrelease["PRETTY_NAME"]).c_str();

				} catch(const std::exception &e) {

					error("Error '{}' reading system release file",e.what());

				}

				//
				// Get machine information
				//
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

					warning("Error '{}' getting DMI information",e.what());

				}

			}

			virtual ~Agent() {
				info("root agent was {}","destroyed");
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
