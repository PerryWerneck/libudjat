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
 #include <udjat/tools/xml.h>
 #include <udjat/agent.h>
 #include <udjat/agent/abstract.h>
 #include <private/randomfactory.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	RandomFactory::RandomFactory() : Abstract::Agent::Factory{"random"} {
		cout << "random agent factory was created" << endl;
		srand(time(NULL));
	}

	std::shared_ptr<Abstract::Agent> RandomFactory::AgentFactory(const Abstract::Agent &parent, const XML::Node &node) const {

		class RandomAgent : public Agent<unsigned int> {
		private:
			unsigned int limit = 5;

		public:
			RandomAgent(const XML::Node &node) : Agent<unsigned int>(node) {
			}

			std::shared_ptr<Abstract::State> computeState() override {

				debug("--------------- Computing state for agent '",name(),"' (",states.size()," state(s) available)");
				for(auto state : states) {
					debug("Checking state '",state->name(),"' with value ",state->value(),"...");
					if(state->compare(get())) {
						debug("State '",state->name(),"' matched with value ",get());
						return state;
					}
					debug("State '",state->name(),"' not matched with value ",get());
				}
				debug("No state matched for agent '",name(),"' with value ",get());
				return Abstract::Agent::computeState();
			}

			bool refresh() override {

				debug("Updating agent '",name(),"'");
				unsigned int last = get();
				unsigned int value = ((unsigned int) rand()) % limit;
				if(value == last) {
					Logger::String{"Current value stayed the same: ",value}.info(name());
					return false;
				}

				Logger::String{"Value changed from ",last," to ",value}.info(name());
				return set(value);
			}

			void start() override {
				Agent<unsigned int>::start( ((unsigned int) rand()) % limit );
			}

		};

		return make_shared<RandomAgent>(node);

	}

 }

