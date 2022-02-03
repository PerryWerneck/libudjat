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
 #include <udjat-internals.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/application.h>

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 #include <cstring>
 #include <unistd.h>

 namespace Udjat {

	void setRootAgent(std::shared_ptr<Abstract::Agent> agent) {

		cout << "agent\tActivating root agent " << hex << ((void *) agent.get()) << endl;
		Abstract::Agent::Controller::getInstance().set(agent);

	}

	std::shared_ptr<Abstract::Agent> getDefaultRootAgent() {

		/// @brief The root agent.
		class Agent : public Abstract::Agent {
		public:
			Agent(const char *name) : Abstract::Agent(name) {

				cout << "agent\tRoot agent " << hex << ((void *) this) << " was created" << endl;
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
					VirtualMachine virtualmachine;

					//
					// Get machine information
					//
					if(!virtualmachine) {

						try {
							URL sysid(Config::Value<string>("bare-metal","summary","dmi:///system/sku"));

							if(!sysid.empty() && Protocol::find(sysid)) {
								this->summary = Quark(sysid.get()).c_str();
							}

						} catch(const std::exception &e) {

							warning("{}",e.what());

						}

					} else {

						this->summary = Quark(Logger::Message("{} virtual machine",virtualmachine.to_string())).c_str();

					}

					if(this->summary && *this->summary) {
						cout << getName() << "\t" << this->summary << endl;
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
				cout << "agent\tRoot agent " << hex << ((void *) this) << " was destroyed" << endl;
			}

			void get(Response &response) override {

				Abstract::Agent::get(response);

#ifdef _WIN32
				OSVERSIONINFO osvi;

				ZeroMemory(&osvi, sizeof(osvi));
				osvi.dwOSVersionInfoSize = sizeof(osvi);

				GetVersionEx(&osvi);

				string sysname{"windows "};

				sysname += to_string(osvi.dwMajorVersion);
				sysname += ".";
				sysname += to_string(osvi.dwMinorVersion);

				sysname += " build ";
				sysname += to_string(osvi.dwBuildNumber);

				sysname += " ";
				sysname += (const char *) osvi.szCSDVersion;

				response["system"] = sysname;

#else
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

		// Get controller to initialize it.
		Abstract::Agent::Controller::getInstance();

		char hostname[255];
		if(gethostname(hostname, 255)) {
			cerr << "agent\tError '" << strerror(errno) << "' getting hostname" << endl;
			strncpy(hostname,Application::Name().c_str(),255);
		}

		return make_shared<Agent>(Quark(hostname).c_str());

	}

 }
