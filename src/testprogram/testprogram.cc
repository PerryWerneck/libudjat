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

 #include <udjat/tools/logger.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/file.h>
 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/module.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/message.h>
 #include <iostream>
 #include <udjat/tools/file.h>
 #include <memory>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fstream>
 #include <fcntl.h>
 #include <iomanip>

#ifdef _WIN32
	#include <udjat/win32/charset.h>
	#include <udjat/win32/exception.h>
	#include <udjat/win32/ip.h>
	#include <udjat/win32/cleanup.h>
#else
	#include <unistd.h>
#endif // _WIN32

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

static const Udjat::ModuleInfo moduleinfo { "Test program" };

static int service_test(int argc, char **argv) {

	class DummyProtocol : public Udjat::Protocol {
	public:
		DummyProtocol() : Udjat::Protocol("dummy",moduleinfo) {
		}

		String call(const URL &url, const HTTP::Method UDJAT_UNUSED(method), const char UDJAT_UNUSED(*payload)) const override {
			cout << "**** dummy\t[" << url << "]" << endl;
			return "";
		}

	};

	class RandomFactory : public Udjat::Factory {
	public:
		RandomFactory() : Udjat::Factory("random",moduleinfo) {
			cout << "random agent factory was created" << endl;
			srand(time(NULL));
		}

		std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object UDJAT_UNUSED(&parent), const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			private:
				unsigned int limit = 5;

			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>(node) {
				}

				bool refresh() override {
					debug("Updating agent '",name(),"'");
					set( ((unsigned int) rand()) % limit );
					return true;
				}

				void start() override {
					Agent<unsigned int>::start( ((unsigned int) rand()) % limit );
				}

			};

			return make_shared<RandomAgent>(node);

		}

	};

	class Service : public SystemService, private RandomFactory {
	private:

		struct {
			DummyProtocol protocol;
		} factories;

	public:
		/// @brief Initialize service.
		void init() override {
			SystemService::init();
		}

		/// @brief Deinitialize service.
		void deinit() override {
			SystemService::deinit();
			Module::unload();
		}

		Service() : SystemService{"./test.xml"} {
		}

	};

	auto rc = Service().run(argc,argv);
	debug("Service exits with rc=",rc);
	Udjat::Module::unload();

	return rc;

}

int main(int argc, char **argv) {

	//cout << Udjat::String{"v1=",1," v2=","2"," v3=",true} << endl;
	//cout << Udjat::Message{"Template v2={2} v1={1} v3={}",1,2,true} << endl;

	// cout <<  Udjat::String{"Searching for second word"}.strcasestr("SECOND") << endl;

	//service_test(argc,argv);

	cout << Application::LogDir() << endl;

	// auto worker = Protocol::WorkerFactory("file://test.xml");
	// worker->save("/tmp/test");

	//File::copy("test.xml","/tmp/test-copy.xml",false);

	//cout << File::Path{"${documents}test"} << endl;

	//Application().shortcut("br.eti.werneck.udjat");

	return 0;
}
