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

 #ifdef _WIN32
	#include <udjat/win32/registry.h>
 #else
	#include <sys/utsname.h>
	#include <unistd.h>
 #endif // _WIN32

 #include <udjat/factory.h>
 #include <udjat/tools/sysconfig.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>
 #include <private/misc.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 #include <cstring>

 namespace Udjat {

	void setRootAgent(std::shared_ptr<Abstract::Agent> agent) {

		cout << "agent\tActivating root agent " << hex << ((void *) agent.get()) << dec << endl;
		Abstract::Agent::Controller::getInstance().set(agent);

	}

	std::shared_ptr<Abstract::Agent> RootAgentFactory() {

		/// @brief The root agent.
		class Agent : public Abstract::Agent {
		public:
			Agent(const char *name) : Abstract::Agent(name) {

				cout << "agent\tRoot agent " << hex << ((void *) this) << dec << " was created" << endl;
				Object::properties.icon = "computer";
				Object::properties.url = Quark(string{"http://"} + name).c_str();

#ifndef _WIN32
				//
				// Get UNAME
				//
				struct utsname uts;

				if(uname(&uts) < 0) {
					memset(&uts,0,sizeof(uts));
					clog << Object::name() << "\tError '" << strerror(errno) << "' getting uts info" << endl;
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

					Object::properties.label = Quark(label).c_str();

				} catch(const std::exception &e) {

					cerr << Object::name() << "\tError '" << e.what() << "' reading system release file" << endl;

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
								Object::properties.summary = Quark(sysid.get()).c_str();
							}

						} catch(const std::exception &e) {

							clog << Object::name() << "\t" << e.what() << endl;

						}

					} else {

						Object::properties.summary = Quark(Logger::Message("{} virtual machine",virtualmachine.to_string())).c_str();

					}

					if(*Object::properties.summary) {
						cout << Object::name() << "\t" << Object::properties.summary << endl;
					}

				}
#endif // HAVE_VMDETECT

			}

			std::shared_ptr<Abstract::State> stateFromValue() const override {

				class ReadyState : public Abstract::State {
				public:
					ReadyState() : Abstract::State("ready", Level::ready, "System is ready", "No abnormal state was detected") {
						Object::properties.icon = "computer";
					}

				};

				return make_shared<ReadyState>();

			}

			virtual ~Agent() {
				info() << "Root agent " << hex << ((void *) this) << dec << " was destroyed" << endl;
			}

			Value & getProperties(Value &value) const noexcept override {

				Abstract::Agent::getProperties(value);

#ifdef _WIN32
				OSVERSIONINFO osvi;

				ZeroMemory(&osvi, sizeof(osvi));
				osvi.dwOSVersionInfoSize = sizeof(osvi);

				GetVersionEx(&osvi);

				string sysname{"windows "};

				sysname += std::to_string(osvi.dwMajorVersion);
				sysname += ".";
				sysname += std::to_string(osvi.dwMinorVersion);

				sysname += " build ";
				sysname += std::to_string(osvi.dwBuildNumber);

				sysname += " ";
				sysname += (const char *) osvi.szCSDVersion;

				value["system"] = sysname;

#else
				struct utsname uts;

				if(uname(&uts) >= 0) {
					value["system"] = string(uts.sysname) + " " + uts.release + " " + uts.version;
				}
#endif // _WIN32

				return value;
			}

			bool activate(std::shared_ptr<Abstract::State> state) noexcept override {

#ifdef DEBUG
				info() << "----------------------------------------------------------" << endl;
#endif // DEBUG
				info() << state->to_string() << endl;

#if defined(HAVE_SYSTEMD)

				sd_notifyf(0,"STATUS=%s",state->to_string().c_str());

#elif defined(_WIN32)

				try {

					Win32::Registry registry("service",true);

					registry.set("status",state->to_string().c_str());
					registry.set("status_time",TimeStamp().to_string().c_str());

				} catch(const std::exception &e) {

					error() << "Error '" << e.what() << "' setting service state" << endl;

				}

#endif // HAVE_SYSTEMD

				if(!state->ready()) {

					// The requested state is not ready, activate it.
					Object::properties.icon = "computer-fail";
					bool changed = Abstract::Agent::activate(state);
					return changed;

				}

				if(this->state()->ready()) {
					// Already ok, do not change.
					return false;
				}

				Object::properties.icon = "computer";
				super::activate(stateFromValue());

				return true;
			}

			void push_back(std::shared_ptr<Abstract::Alert> alert) {

				alert->info() << "Root alert, emitting it on startup" << endl;

				// Can't start now because the main loop is not active, wait 100ms.
				MainLoop::getInstance().insert(0,100,[alert](){
					auto activation = alert->ActivationFactory();
					Udjat::start(activation);
					return false;
				});

			}

		};

		// Get controller to initialize it.
		Abstract::Agent::Controller::getInstance();

		char hostname[256];
		if(gethostname(hostname, 255)) {
			cerr << "agent\tError '" << strerror(errno) << "' getting hostname" << endl;
			strncpy(hostname,Application::Name().c_str(),255);
		}

		return make_shared<Agent>(Quark(hostname).c_str());

	}

 }
