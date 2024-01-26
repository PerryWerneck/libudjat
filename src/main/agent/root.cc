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
 #include <private/agent.h>

 #ifdef _WIN32
	#include <udjat/win32/registry.h>
 #else
	#include <sys/utsname.h>
	#include <unistd.h>
 #endif // _WIN32

 #include <udjat/tools/factory.h>
 #include <udjat/tools/sysconfig.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/intl.h>
 #include <private/misc.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/network.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/alert/activation.h>
 #include <udjat/module/abstract.h>
 #include <sstream>

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 #include <cstring>

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> Abstract::Agent::RootFactory() {

		/// @brief The root agent.
		class Agent : public Abstract::Agent {
		private:
			std::vector<std::shared_ptr<Abstract::State>> states;

		public:
			Agent(const char *name) : Abstract::Agent(name) {

				{
					std::stringstream ss;
					ss << "Building root agent " << hex << ((void *) this) << dec;
					Logger::String{ss.str()}.trace("agents");
				}

				Object::properties.label = Quark(Hostname()).c_str();

				//
				// Create default 'ready' state.
				//
				class ReadyState : public Abstract::State {
				public:
					ReadyState(const char *name, const char *summary, const char *body) : Abstract::State(name, Level::ready, summary, body) {
#ifdef HAVE_VMDETECT
						VirtualMachine virtualmachine;
						Object::properties.icon = virtualmachine ? "computer-vm" : "computer";
#else
						Object::properties.icon = "computer";
#endif // HAVE_VMDETECT
					}

				};

				auto ready = make_shared<ReadyState>(name, _( "System is ready" ), _( "No abnormal state was detected" ));
				Object::properties.icon = ready->icon();

				states.push_back(ready);

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
							URL sysid{Config::Value<string>("bare-metal","summary","dmi:///system/sku").c_str()};

							if(!sysid.empty() && Protocol::find(sysid)) {
								Object::properties.summary = Quark(sysid.get()).c_str();
							}

						} catch(const std::exception &e) {

							clog << Object::name() << "\t" << e.what() << endl;

						}

					} else {

						Object::properties.summary = Quark(Logger::Message("{} virtual machine",virtualmachine.name())).c_str();

					}

					if(*Object::properties.summary) {
						cout << Object::name() << "\t" << Object::properties.summary << endl;
					}

				}
#endif // HAVE_VMDETECT

			}

			std::shared_ptr<Abstract::State> StateFactory(const XML::Node &node) override {

				auto state = make_shared<Abstract::State>(node);

				// Keep only one state for every level.
				for(auto it = states.begin(); it != states.end(); it++) {

					if((*it)->level() == state->level()) {
						states.erase(it);
						break;
					}

				}

				states.push_back(state);
				return state;
			}

			std::shared_ptr<Abstract::State> computeState() override {

				for(auto state : states) {
					if(state->ready()) {
						return state;
					}
				}

				return super::computeState();
			}

			virtual ~Agent() {
				std::stringstream ss;
				ss << "Root agent " << hex << ((void *) this) << dec << " was destroyed";
				Logger::String{ss.str()}.trace("agents");
			}

			Value & getProperties(Value &value) const override {

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

			bool set(std::shared_ptr<Abstract::State> state) noexcept override {

				auto level = state->level();
				if(level < Level::ready) {
					level = Level::ready;
				}

				for(auto st : states) {
					if(st->level() == level) {
						state = st;
						break;
					}
				}

				Object::properties.icon = (state->ready() ? "computer" : "computer-fail");

				return super::set(state);

			}

			void push_back(std::shared_ptr<Activatable> activatable) override {

				// Can't start now because the main loop is not active, wait 100ms.
				MainLoop::getInstance().TimerFactory(100,[this,activatable](){

					activatable->info() << "Orphaned action, activating on startup" << endl;

					Abstract::Alert *alert = dynamic_cast<Abstract::Alert *>(activatable.get());
					if(alert) {
						auto activation = alert->ActivationFactory();
						activation->set(*Abstract::Agent::root());
						Udjat::start(activation);
					} else {
						activatable->activate(*this);
					}
					return false;
				});

			}

		};

		// Get controller to initialize it.
		Abstract::Agent::Controller::getInstance();

		return make_shared<Agent>(Application::Name::getInstance().c_str());

	}

 }
