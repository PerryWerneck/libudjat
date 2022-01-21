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

 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <udjat/alert.h>
 #include <iostream>
 #include <memory>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	class RandomFactory : public Udjat::Factory {
	private:

		struct {
			Alert::Factory alert;
		} factories;

	public:
		RandomFactory() : Udjat::Factory("random") {
			cout << "random agent factory was created" << endl;
			srand(time(NULL));
		}

		bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			private:
				unsigned int limit = 5;

			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>() {
					cout << "Creating random Agent" << endl;
					load(node);
				}

				bool refresh() override {
					set( ((unsigned int) rand()) % limit);
					return true;
				}

			};

			parent.insert(make_shared<RandomAgent>(node));

			return true;
		}

	};

	class Service : public SystemService, private RandomFactory {
	public:
		/// @brief Initialize service.
		void init() override {
			cout << Application::Name() << "\tInitializing" << endl;
			// Alert::initialize();

			auto root = Abstract::Agent::init("*.xml");

			cout << "http://localhost:8989/api/1.0/info/modules.xml" << endl;
			cout << "http://localhost:8989/api/1.0/info/workers.xml" << endl;
			cout << "http://localhost:8989/api/1.0/info/factories.xml" << endl;
			cout << "http://localhost:8989/api/1.0/alerts.xml" << endl;
			cout << "http://localhost:8989/api/1.0/agent.xml" << endl;

			for(auto agent : *root) {
				cout << "http://localhost:8989/api/1.0/agent/" << agent->getName() << ".xml" << endl;
			}

			/*
			MainLoop::getInstance().insert(this,5000,[]() {
				MainLoop::getInstance().quit();
				return false;
			});
			*/

			Alert::activate("test","http://localhost");

		}

		/// @brief Deinitialize service.
		void deinit() override {
			cout << Application::Name() << "\tDeinitializing" << endl;
		}

		Service() = default;


	};


	/*
#ifdef _WIN32
#endif // _WIN32
	*/

	return Service().run(argc,argv);


}
