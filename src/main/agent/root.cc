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
 #include <cstring>
 #include <unistd.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-bus.h>
 #endif // HAVE_SYSTEMD

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
				string virtualization;

#ifdef HAVE_SYSTEMD
				{
					// Detected virtualization with systemd.

					// Reference: (https://www.freedesktop.org/software/systemd/man/org.freedesktop.systemd1.html)
					sd_bus * bus = NULL;
					if(sd_bus_open_system(&bus) < 0) {

						clog << getName() << "\tCant open system bus" << endl;

					} else {

						/*
							dbus-send \
								--session \
								--dest=org.freedesktop.systemd1 \
								--print-reply \
								"/org/freedesktop/systemd1" \
								"org.freedesktop.DBus.Properties.Get" \
								string:"org.freedesktop.systemd1.Manager" \
								string:"Virtualization"
						*/

						sd_bus_error error = SD_BUS_ERROR_NULL;
						sd_bus_message *response = NULL;

						int rc = sd_bus_call_method(
								bus,                   					// On the System Bus
                                "org.freedesktop.systemd1",				// Service to contact
                                "/org/freedesktop/systemd1", 			// Object path
                                "org.freedesktop.DBus.Properties",		// Interface name
                                "Get",									// Method to be called
                                &error,									// object to return error
                                &response,								// Response message on success
                                "ss",
                                "org.freedesktop.systemd1.Manager",
                                "Virtualization"
						);

						if(rc < 0) {

							cerr << getName() << "\t" << error.message << endl;
							sd_bus_error_free(&error);

						} else if(response) {

							char *text = NULL;

							// It's a variant

							if(sd_bus_message_read(response, "v", "s", &text) < 0) {
								cerr << getName() << "\tCan't parse systemd virtualization response" << endl;
							} else if(text && *text) {
								cout << getName() << "\t Detected '" << text << "' virtualization" << endl;
								virtualization = text;
							}

							sd_bus_message_unref(response);

						}

						sd_bus_flush_close_unref(bus);

					}

				}
#endif // HAVE_SYSTEMD

				//
				// Get machine information
				//
				if(virtualization.empty()) {

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
				} else {

					string text{"{} virtual machine"};
					size_t pos = text.find("{}");
					if(pos != string::npos) {
						text.replace(pos,2,virtualization);
					}

					this->summary = Quark(text).c_str();

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

		};

		char hostname[255];
		if(gethostname(hostname, 255)) {
			cerr << "Error '" << strerror(errno) << "' getting hostname" << endl;
			strncpy(hostname,PACKAGE_NAME,255);
		}

		return make_shared<Agent>(Quark(hostname).c_str());

	}

 }
