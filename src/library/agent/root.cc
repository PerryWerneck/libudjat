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

 #include <udjat/module/abstract.h>
 #include <udjat/tools/system.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/intl.h>
 #include <private/misc.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/network.h>
 #include <udjat/tools/response.h>
 #include <udjat/alert.h>
 #include <udjat/module/abstract.h>
 #include <udjat/agent/abstract.h>
 #include <sstream>

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 #ifdef HAVE_SMBIOS
	#include <smbios/value.h>
 #endif // HAVE_SMBIOS

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
					}

					const char * icon() const noexcept override {
#ifdef HAVE_VMDETECT
						if(VirtualMachine{Logger::enabled(Logger::Debug)}) {
							return "computer-vm";
						}
#endif // HAVE_VMDETECT
						return "computer";
					}

				};

				auto ready = make_shared<ReadyState>(name, _( "System is ready" ), _( "No abnormal state was detected" ));
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

					string label = System::Config::File{"/etc/os-release","="}["PRETTY_NAME"].value;
					if(uts.machine[0]) {
						label += " ";
						label += uts.machine;
					}

					Object::properties.label = Quark(label).c_str();

				} catch(const std::exception &e) {

					cerr << Object::name() << "\tError '" << e.what() << "' reading system release file" << endl;

				}
#endif // _WIN32

			}

			const char * summary() const noexcept override {
#ifdef HAVE_VMDETECT
				debug("---");
				VirtualMachine vm{Logger::enabled(Logger::Debug)};
				if(vm) {
					return Quark{Logger::Message("{} virtual machine",vm.name())}.c_str();
				}
#endif // HAVE_VMDETECT
				return super::summary();
			}

			void start() override {

				if(Object::properties.summary && *Object::properties.summary) {
					return;
				} else {

					try {
						
						String sysid = URL{Config::Value<string>{"bare-metal","summary","dmi:///system/sku"}.c_str()}.get();

						Object::properties.summary = sysid.split("\n",2)[0].as_quark();

					} catch(const std::exception &e) {

						Logger::String{e.what()}.trace();

					}

				}

				super::start();

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

		};

		// Get controller to initialize it.
		Abstract::Agent::Controller::getInstance();

		return make_shared<Agent>(Application::Name::getInstance().c_str());

	}

 }
