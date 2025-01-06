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

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo { "Test program" };

 int main(int argc, char **argv) {
	return Testing::run(argc,argv,moduleinfo,[](Udjat::Application &){

		Logger::String{"----> System CPE is '",Udjat::System::cpe().c_str(),"'"}.trace();

		Agent<unsigned short> test{"test-agent",XML::Node{}};

		{
			Agent<Percentage> percent{"test-percent",0.1};

			string str;
			percent.getProperty("value",str);
			debug("PERCENT------------------> ",str.c_str());

			debug("Expanding----> '",String{"The percent value is ${value}"}.expand(percent).c_str(),"'");
			
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
