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
 #include <udjat/agent/abstract.h>
 #include <udjat/agent.h>
 #include <udjat/agent/percentage.h>
 #include <udjat/tools/url.h>
 #include <udjat/net/ip/address.h>

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo { "Test program" };

 int main(int argc, char **argv) {

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
			debug("URL----------------------> ",url.c_str());
				
		}

		MainLoop::getInstance().TimerFactory(1000,[]{

			// cout << "-[ On Timer ]---------------------------------------------------" << endl;
			// Request request{"/"};
			// Response response{Udjat::html};			
			// Interface::find("agent").call(request,response);
			// auto &report = response["resultset"].ReportFactory("a","b","c",nullptr);
			// report.push_back("String");
			// report.push_back(TimeStamp{});
			// for(unsigned ix = 0; ix < 10;ix++) {
			//	report.push_back(ix);
			// }
			//cout << "Response:" << endl << response << endl;
			// cout << "----------------------------------------------------------------" << endl;
			
			return false;
		});

	});

 }
