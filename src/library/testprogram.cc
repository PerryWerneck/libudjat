/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 #if defined(DEBUG) and ! defined(LIBUDJAT_STATIC) 

 #include <udjat/defs.h>
 #include <udjat/defs.h>
 #include <udjat/tools/unit-test.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/schema.h>
 #include <udjat/agent.h>
 #include <ostream>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 UDJAT_API void enum_udjat_unit_tests(Udjat::UnitTests &tests) noexcept {

	tests.append(
		UnitTests::Worker{
			"interface", "Interface test",
			[](std::ostream &stream) {

				// Check module with extra path.
				{
					const char *request = "/module/test";
					auto intf = Interface::find(request);
					if(!intf) {
						throw runtime_error("Cant find interface for /module");
					}
					if(strcmp(request,"/test")) {
						throw runtime_error("Unexpected result after request parse");
					}
				}

				// Check without extra path.
				{
					const char *request = "/module";
					auto intf = Interface::find(request);
					if(!intf) {
						throw runtime_error("Cant find interface for /module");
					}
					if(request[0]) {
						throw runtime_error("Unexpected result after request parse");
					}
				}

				// Check API (request/response)
				{
					const char *path = "/module";
					auto intf = Interface::find(path);
					if(!intf) {
						throw runtime_error("Cant find interface for /module");
					}

					Request request{path};
					Response response;

					if(!intf->process(path,request,response)) {
						throw runtime_error("Request /module was not processed");
					}

					cout << endl << "Output:" << endl;
					response.serialize(cout);
					cout << endl;
				}

				return "Interface test passed";
			}
		},
		UnitTests::Worker{
			"agent", "Basic agent tests",
			[](std::ostream &stream) {

				Agent<int> agent;
				Schema schema;

				if(!agent.output_schema("",schema)) {
					throw runtime_error("Agent should have schema");
				}

				stream << "Got agent schema:" << endl;
				for(const auto &item : schema) {
					stream 	<< "\t"
							<< item.name()
							<< "\t"
							<< std::to_string(item.type())
							<< "\t"
							<< item.description()
							<< endl;
				}
				stream << endl;

				return "Basic agent tests passed";
			}
		}
	);

 }

 #endif // DEBUG

 