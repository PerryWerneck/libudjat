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

 // #define SERVICE_TEST 1
 #define APPLICATION_TEST 1
 // #define OBJECT_TEST 1

 #include <config.h>

 #include <udjat/tools/logger.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/intl.h>
 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/message.h>
 #include <udjat/tools/file/watcher.h>
 #include <iostream>
 #include <udjat/tools/file.h>
 #include <memory>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fstream>
 #include <fcntl.h>
 #include <iomanip>
 #include <udjat/net/ip/address.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/xml.h>
 #include <udjat/ui/icon.h>

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

 /*
 class DummyProtocol : public Udjat::Protocol {
 public:
	DummyProtocol() : Udjat::Protocol("dummy",moduleinfo) {
	}

	String call(const URL &url, const HTTP::Method UDJAT_UNUSED(method), const char UDJAT_UNUSED(*payload)) const override {
		cout << "**** dummy\t[" << url << "]" << endl;
		return "";
	}

 };
 */

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

#if defined(SERVICE_TEST)
int main(int argc, char **argv) {

	class Service : public SystemService, private RandomFactory {
	public:

		void root(std::shared_ptr<Abstract::Agent> agent) override {
			debug("--------------------------------> ",agent->name()," is the new root");
			SystemService::root(agent);
		}

	};

	Logger::verbosity(9);

	/*
	{
		SubProcess{"test","echo su - root --login -- /usr/sbin/grub2-reboot \"\\\"Reinstalar estação de trabalho\\\"\""}.run();
		return 0;
	}
	*/

	/*
	MainLoop::getInstance().TimerFactory(1000,[]{
		debug("---------------------------------------------------------------------");
		SubProcess{"test","ls -l",Logger::Warning}.run();
		debug("---------------------------------------------------------------------");
		return false;
	});
	*/


	//DummyProtocol protocol;
	auto rc = Service{}.run(argc,argv,"./test.xml");

	debug("Service exits with rc=",rc);

	return rc;

}
#elif defined(APPLICATION_TEST)
int main(int argc, char **argv) {

	class Application : public Udjat::Application {
	public:

		int install(const char *name) override {
			ShortCut{}.desktop();
			return super::install(name);
		}

		int uninstall() override {
			ShortCut{}.remove();
			return super::uninstall();
		}

		void root(std::shared_ptr<Abstract::Agent> agent) override {
			debug("--------------------------------> ",agent->name()," is the new root");
			debug("test-arg='",getProperty("test-arg","default"));
			debug("-------------------------------------------------------");
			new File::Watcher("test.xml");
			debug("-------------------------------------------------------");
		}

	};

	Logger::verbosity(9);
	Logger::redirect();


	// DummyProtocol protocol;
	auto rc = Application{}.run(argc,argv,"./test.xml");

	debug("Application exits with rc=",rc);

	return rc;

}
#endif // defined

#if defined(OBJECT_TEST)
int main(int argc, char **argv) {

	bindtextdomain(GETTEXT_PACKAGE, STRINGIZE_VALUE_OF(LOCALEDIR));
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	debug("Locale set to ",STRINGIZE_VALUE_OF(LOCALEDIR),"/",GETTEXT_PACKAGE);
	Logger::redirect();
	Logger::verbosity(9);
	Logger::console(true);

	/*
	printf("------------------------\n");
	cout << String{}.set_byte(10000.0) << endl;
	cout << String{}.set_byte(0.0) << endl;
	cout << String{}.set_byte(229.0) << endl;
	printf("------------------------\n");
	*/

	/*
	URL url{"http://example.com?v1=10&v2=20"};

	cout << "sz=" << url[url.size()-1] << endl;
	cout << "Arg v1='" << url["v1"] << "'" << endl;
	cout << "Arg v2='" << url["v2"] << "'" << endl;
	*/

	/*
	String test{"create table if not exists urls\n(id integer primary key, inserted timestamp default CURRENT_TIMESTAMP, url text, action text, payload text)"};

	std::vector<String> lines = test.split("\n");

	for(auto &line : lines) {
		cout << "Line: '" << line << "'" << endl;
	}
	*/

	// File::Path{"/usr/share/icons/gnome/"}.find("*/computer-symbolic.svg",true);

	/*
	Request request{"/1/2/3/4"};

	for(size_t ix = 1;ix < 5; ix++) {
		String s{request[ix]};
		cout << "Arg[" << ix << "]='" << s << "'" << endl;
		cout << "pop='" << request.pop() << "'" << endl << endl;
	}
	*/

	/*
	{
		String text{"+Hello cruel world+/How are you today?"};

		cout << "Escaped:   '" << text.escape() << "'" << endl;
		cout << "Unescaped: '" << text.unescape() << "'" << endl;
	}
	*/

	/*
	{
		enum Test{ v1, v2, v3 };

		Udjat::Agent<Test> ag{"test",v1};

	}
	*/

	/*
	File::Path file{"/tmp/xpto.txt"};
	cout << file.name() << endl;
	*/

	/*
	string name = Udjat::Icon{"computer"}.filename();
	cout << "Found: '" << name << "'" << endl;
	*/


	return 0;
}
#endif // OBJECT_TEST
