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
 #include <udjat/tools/system.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/report.h>
 #include <udjat/tools/logger.h>	
 #include <udjat/agent/abstract.h>
 #include <udjat/agent.h>
 #include <udjat/agent/percentage.h>
 #include <udjat/tools/url.h>
 #include <udjat/net/ip/address.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/ui/console.h>
 #include <udjat/ui/progress.h>
 #include <private/dialog.h>
 #include <udjat/net/ip/address.h>
 #include <udjat/net/interface.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo { "Test program" };

 int main(int argc, char **argv) {

	Logger::verbosity(9);

	Config::for_each("keys", [](const char *name, const char *value) ->bool {
		debug(name,"=",value);
		return false;
	});

	exit(0);

	XML::load("test.xml");

	/*
	cout << "---------------> GATEWAY=" << IP::gateway().to_string() << endl;
	cout << "---------------> INTERFACE=" << Network::Interface::Default()->address().to_string() << endl;
	{
		auto dialog = Dialog::Progress::getInstance();
		dialog->item(1,1);
		dialog->set("http://www.google.com");

		for(size_t ix = 0; ix < 1000; ix++) {
			dialog->set(ix,1000);
			usleep(1500);
		}

	}

	debug("Sleeping");
	sleep(5);
	debug("Complete");
	*/

	/*
	{

		UI::Console console;
		console << "Testing simple line writer" << endl;
		console << endl << endl << endl;
		console << UI::Console::Foreground::Green;
		console.bold(true);
		console.italic(true);
		for(size_t ix = 0; ix < 1000; ix++) {
			console.progress("(001/001)","http://www.google.com",ix,1000);
			usleep(1500);
		}
		console.faint(false);
		console << endl << endl << endl;
	}
	*/

	/*
	{
		Config::Value<string> config{"test-config", "test-value","default-value"};
		debug("Config value: ",config.c_str()," len=",config.size()," value[0]=",config[0]);
	}
	*/


	/*
	debug("Argc=",argc);
	*/

	/*
	return Testing::run(argc,argv,moduleinfo,[](Udjat::Application &){

		Logger::String{"----> System CPE is '",Udjat::System::cpe().c_str(),"'"}.trace();

		Agent<unsigned short> test{"test-agent",XML::Node{}};

		debug("-------------------------")
		debug("NIC=",IP::Address{"192.168.0.17"}.nic().c_str());
		debug("MAC=",IP::Address{"192.168.0.17"}.macaddress().c_str());

		{
			String str{"line1,line2, line3 , line4"};
	
			debug("Split test:");
			auto lines = str.split(",");
			for(auto &line : lines) {
				debug("LINE----> '",line.c_str(),"'");
			}
			if(lines.size() != 4) {
				throw runtime_error("Split failed");
			}

			debug("Split with length test:");
			lines = str.split(",",2);
			for(auto &line : lines) {
				debug("LINE----> '",line.c_str(),"'");
			}
			if(lines.size() != 2) {
				throw runtime_error("Split with length failed");
			}

		}

		{
			String test{"The value 1:6 is '${value[1:6]}'"};
			
			test.expand([&test](const char *key, std::string &str){
				if(!strcasecmp(key,"value")) {
					str = "0123456789abcdefghijklmno";
					return true;
				}
				return false;
			});

			debug("result='",test.c_str(),"'");

			if(strcmp(test.c_str(),"The value 1:6 is '12345'") != 0) {
				throw runtime_error("Expand failed");
			}

		}

		{
			Agent<Percentage> percent{"test-percent",0.1};

			string str;
			percent.getProperty("value",str);
			debug("PERCENT------------------> ",str.c_str());

			debug("Expanding----> '",String{"The percent value is ${value}"}.expand(percent).c_str(),"'");
			
		}

		{
			debug("-------------------> String split");
			String test{"create table if not exists alerts (id integer primary key, url text, action text, payload text)"};

			std::vector<String> lines = test.split("\n");
			debug("----> '",lines[0].c_str(),"'");

			if(lines[0].size() != test.size()) {
				throw runtime_error("Split failed");
			}

		}

		{
			debug("-------------------> URL test");
			URL url{"http://www.google.com:8080/path/to/somewhere?name=value&name2=value2"};
			debug("URL----------------------> ",url.c_str());
			debug("Hostname----------------> ",url.hostname().c_str());
			debug("Scheme------------------> ",url.scheme().c_str());
			debug("Path--------------------> ",url.path().c_str());

			url += "./another/path";
			
			if(strcmp(url.c_str(),"http://www.google.com:8080/path/to/somewhere/another/path?name=value&name2=value2")) {
				throw runtime_error("URL append failed");
			} 
				
		}


	});
	*/

 }
