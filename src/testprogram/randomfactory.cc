/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tests.h>
 #include <udjat/tools/factory.h>
 #include <udjat/tools/xml.h>
 #include <udjat/agent.h>
 #include <udjat/agent/abstract.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	Testing::RandomFactory::RandomFactory(const Udjat::ModuleInfo &info) : Udjat::Factory("random",info) {
		cout << "random agent factory was created" << endl;
		srand(time(NULL));
	}

	std::shared_ptr<Abstract::Agent> Testing::RandomFactory::AgentFactory(const Abstract::Object &, const XML::Node &node) const {

		class RandomAgent : public Agent<unsigned int> {
		private:
			unsigned int limit = 5;

		public:
			RandomAgent(const XML::Node &node) : Agent<unsigned int>(node) {
			}

			bool refresh() override {
				debug("Updating agent '",name(),"'");
				unsigned int value = ((unsigned int) rand()) % limit;
				if(set(value)) {
					Logger::String{"Agent '",name(),"' updated to ",value}.info();
					return true;
				}
				Logger::String{"Agent '",name(),"' keep value ",value}.info();
				return false;
			}

			void start() override {
				Agent<unsigned int>::start( ((unsigned int) rand()) % limit );
			}

		};

		return make_shared<RandomAgent>(node);

	}

 }

