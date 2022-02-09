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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <udjat/alert.h>
 #include <udjat/module.h>
 #include <udjat/tools/url.h>
 #include <iostream>
 #include <udjat/tools/file.h>
 #include <memory>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

static const Udjat::ModuleInfo moduleinfo { "Test program" };

int main(int argc, char **argv) {

	class DummyProtocol : public Udjat::Protocol {
	public:
		DummyProtocol() : Udjat::Protocol("dummy",moduleinfo) {
		}

		std::string call(const URL &url, const HTTP::Method method, const char *payload) const override {
			cout << "dummy\t" << url << endl;
			return "";
		}

	};

	class RandomFactory : public Udjat::Factory {
	public:
		RandomFactory() : Udjat::Factory("random",moduleinfo) {
			cout << "random agent factory was created" << endl;
			srand(time(NULL));
		}

		bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			private:
				unsigned int limit = 5;

			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>(node) {
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
	private:

		struct {
			Alert::Factory alert;
			DummyProtocol protocol;
		} factories;

	public:
		/// @brief Initialize service.
		void init() override {

			SystemService::init();

			if(Module::find("httpd")) {
				if(Module::find("information")) {
					cout << "http://localhost:8989/api/1.0/info/modules.xml" << endl;
					cout << "http://localhost:8989/api/1.0/info/workers.xml" << endl;
					cout << "http://localhost:8989/api/1.0/info/factories.xml" << endl;
				}
				cout << "http://localhost:8989/api/1.0/alerts.xml" << endl;

				{
					auto root = Udjat::Abstract::Agent::root();
					if(root) {
						cout << "http://localhost:8989/api/1.0/agent.xml" << endl;
						for(auto agent : *root) {
							cout << "http://localhost:8989/api/1.0/agent/" << agent->name() << ".xml" << endl;
						}
					}
				}

			}


#ifdef _WIN32
			{
				HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

				cout << "Creating event " << hex << ((unsigned long long) hEvent)<< dec << endl;

				MainLoop::getInstance().insert(hEvent,[](HANDLE handle, bool abandoned){

					if(abandoned) {
						cout << "event\tEvent was abandoned" << endl;
					} else {
						cout << "event\tSignaled" << endl;
					}

				});

				MainLoop::getInstance().insert(this,1000,[hEvent]() {
					static int counter = 5;
					cout << "timer\tEvent " << hex << ((unsigned long long) hEvent) << dec << " (" << counter << ")" << endl;
					if(--counter > 0) {
						SetEvent(hEvent);
					} else {
						MainLoop::getInstance().remove(hEvent);
						CloseHandle(hEvent);
						return false;
					}
					return true;
				});
			}
#endif // _WIN32

			//Alert::activate("test","dummy+http://localhost");

		}

		/// @brief Deinitialize service.
		void deinit() override {
			cout << Application::Name() << "\tDeinitializing" << endl;
			Module::unload();
		}

		Service() : SystemService{"./test.xml"} {
		}

	};

	Config::for_each("service-events",[](const char *key, const char *value){
		cout << "config\t" << key << "='" << value << "'" << endl;
		return true;
	});

	cout << "webroot: '" << Application::DataDir("www/error-pages") << endl;

	{
		File::Temporary tempfile("test.tmp");

		tempfile.write("teste\n");
		tempfile.save();

	}

	return Service().run(argc,argv);

}
