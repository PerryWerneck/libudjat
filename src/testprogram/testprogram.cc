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
 #include <iostream>
 #include <udjat/tools/file.h>
 #include <memory>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fstream>
 #include <fcntl.h>

#ifdef _WIN32
	#include <udjat/win32/charset.h>
	#include <udjat/win32/exception.h>
#else
	#include <unistd.h>
#endif // _WIN32

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

static const Udjat::ModuleInfo moduleinfo { "Test program" };

int main(int argc, char **argv) {

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

			if(Module::find("httpd")) {

				if(Module::find("information")) {
					debug("http://localhost:8989/api/1.0/info/modules.xml");
					debug("http://localhost:8989/api/1.0/info/workers.xml");
					debug("http://localhost:8989/api/1.0/info/factories.xml");
				}
				debug("http://localhost:8989/api/1.0/alerts.xml");

				{
					auto root = Udjat::Abstract::Agent::root();
					if(root) {
						debug("http://localhost:8989/api/1.0/agent.html");
						for(auto agent : *root) {
							debug("http://localhost:8989/api/1.0/agent/",agent->name(),".html");
						}
					}
				}

			}

			// Protocol::call("dummy+http://localhost");

			/*
			MainLoop::getInstance().insert(0,2000,[](){
				MainLoop::getInstance().quit();
				return false;
			});
			*/

			/*
			MainLoop::getInstance().insert(0,2000,[](){
				cout << "------------------------------------------" << endl;
				cout << "Cache: " << URL("http://localhost").filename() << endl;
				cout << "------------------------------------------" << endl;
				return false;
			});
			*/

			cout << "test\táéióú" << endl;

			MainLoop::getInstance().TimerFactory(2000,[](){
#ifdef _WIN32
				SubProcess::start("subprocess.bat");
#else
				SubProcess::start("ls");
#endif // _WIN32
				return false;
			});
			cout << "------------------------------------------" << endl;

#ifdef _WIN32
			/*
			{
				HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

				cout << "Creating event " << hex << ((unsigned long long) hEvent)<< dec << endl;

				MainLoop::getInstance().insert(hEvent,[](HANDLE handle, bool abandoned){

					if(abandoned) {
						cout << "event\tEvent was abandoned" << endl;
					} else {
						cout << "event\tSignaled" << endl;
					}

					return true;
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
			*/
#endif // _WIN32

			//Alert::activate("test","dummy+http://localhost");

		}

		/// @brief Deinitialize service.
		void deinit() override {
			SystemService::deinit();
			Module::unload();
		}

		Service() : SystemService{"./test.xml"} {
		}

	};

	// File::copy(argv[0],"/tmp/test");

	// File::List(Application::DataDir("icons"),true);
	// File::List("/usr/share/icons/","*.png",true);
	//if(File::Path("/usr/share/icons").find("window-new-symbolic.svg",true)) {
	//	cout << "FOUND!!!" << endl;
	//}
	//return 0;

	/*
	{
		URL url{"http://host.domain/sample"};

		url += "newpath";
		cout << "simple add: '" << url << "'" << endl;

		url += "../otherpath";
		cout << "upsearch add: '" << url << "'" << endl;

		url += "../../thepath";
		cout << "double upsearch add: '" << url << "'" << endl;

		url = "https://download.opensuse.net.br/distribution/leap/15.4/repo/oss";
		url += "/boot/x86_64/loader/linux";

		cout << "Simple concat: '" << url << "'" << endl;

		return 0;
	}
	*/

	/*
#ifdef _WIN32
	return SubProcess("subprocess.bat").run();
#else
	return SubProcess("ls").run();
#endif // _WIN32
	*/

	/*
	{
		Application::CacheDir cache{"urls"};
		cout << "Cache set to " << cache << endl;
	}
	*/

	/*
	{
		cout << "----------------------------" << endl;
		cout << "Charset=" << Win32::Charset::system() << endl;

		// Udjat::File::Text text("charset.txt");
		std::string text{"áéíóú"};

		{
			ofstream file;
			file.open("source.txt", std::ios::binary);
			file << text.c_str() << endl;
		}

		{
			ofstream file;
			file.open("to_windows.txt", std::ios::binary);
			file << Win32::Charset::to_windows(text.c_str()).c_str() << endl;
		}

		{
			ofstream file;
			file.open("from_windows.txt", std::ios::binary);
			file << Win32::Charset::from_windows(text.c_str()).c_str() << endl;
		}

		cout << "----------------------------" << endl;
	}
	*/

/*
#ifdef _WIN32
	{
		Logger::redirect(true,true);
		Application::InstallLocation appinstall;
		debug("AppInstall=",appinstall.c_str());
		if(appinstall) {
			cout << "InstalLocation='" << appinstall << "'" << endl;
		} else {
			cout << "No install location" << endl;
		}
	}

#endif // _WIN32
*/

	// debug("SystemDatadir=",Application::SystemDataDir().c_str());
	// debug("CacheDir=",Application::CacheDir().c_str());
	// debug("LogDir=",Application::LogDir().c_str());
	// debug("Timestamp=",TimeStamp(90000).to_verbose_string());


	auto rc = Service().run(argc,argv);

	debug("Service exits with rc=",rc);

	Udjat::Module::unload();

	return rc;
}
