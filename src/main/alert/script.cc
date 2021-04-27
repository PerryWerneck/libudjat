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
 #include <udjat/tools/subprocess.h>
 #include <udjat/factory.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>

  namespace Udjat {

	ScriptAlert::ScriptAlert(const pugi::xml_node &node) : Alert(node) {

		string section = getConfigSection(node,"script");

		command =
			Attribute(node,"script",false)
				.as_string(
					Config::Value<string>(section.c_str(),"script",command.c_str()).c_str()
				);


	}

	ScriptAlert::~ScriptAlert() {
	}

	void ScriptAlert::activate(const Abstract::Agent &agent, const Abstract::State &state) {

		class Event : public Alert::Event {
		public:

			string command;
			string name;

			Event(const Abstract::Agent &agent, const Abstract::State &state,string &s) : Alert::Event(agent, state),command(s),name(agent.getName()) {
				info("Event '{}' created",command);
			}

			virtual ~Event() {
				info("Event '{}' destroyed",command);
			}

			const char * getDescription() const override {
				return command.c_str();
			}

			void alert(size_t current, size_t total) override {

				info("Starting '{}' ({}/{})",this->command,current,total);

				class Script : public SubProcess {
				private:
					string name;

				public:
					Script(const string &n, const char *command) : Udjat::SubProcess(command), name(n) {
					}

					virtual ~Script() {
					}

					void onStdOut(const char *line) override {
						cout << name << "\t" << line << endl;
					}

					void onStdErr(const char *line) override {
						cerr << name << "\t" << line << endl;
					}

				};

				(new Script(name,command.c_str()))->start();

			}

		};

		// Expand command line.
		string command = this->command.c_str();
		agent.expand(command);
		state.expand(command);

		// Create event.
		auto event = make_shared<Event>(agent,state,command);

		// Activate event.
		Alert::activate(event);
	}


 }
