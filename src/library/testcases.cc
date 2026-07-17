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
 #include <udjat/tools/testsuite.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/application.h>
 #include <udjat/agent.h>
 #include <udjat/tools/http/mimetype.h>
 #include <ostream>
 #include <sstream>
 #include <stdexcept>
 #include <iomanip>
 #include <private/agent.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 UDJAT_API void udjat_register_tests(Udjat::TestSuite &suite) noexcept {

	suite.add(
		TestSuite::Case{
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
					Response response{MimeType::yaml};

					if(!intf->process(path,request,response)) {
						throw runtime_error("Request /module was not processed");
					}

					stream << endl << "Output:" << endl;
					response.serialize(stream);
					stream << endl;
				}

				return "Interface test passed";
			}
		},
		TestSuite::Case{
			"agent", "Basic agent tests",
			[](std::ostream &stream) {

				{
					// Test agent properties.
					Agent<int> agent{};
					Schema schema;

					if(!agent.output_schema("",schema)) {
						throw runtime_error("Agent should have schema");
					}

					stream << "Got agent schema:" << endl;
					for(const auto &item : schema) {
						stream 	<< "   "
								<< left
								<< setw(15) << item.name()
								<< "   "
								<< setw(10) << std::to_string(item.type())
								<< "   "
								<< item.description()
								<< endl;
					}
					stream << endl;

					Request request;
					Response response{MimeType::yaml};

					agent.process("",request,response);

					stream	<< "Got agent response:" 
							<< endl
							<< response
							<< endl;

					Value value;
					agent.get_properties(value);

					stream	<< "Got agent properties:" 
							<< endl
							<< value.serialize(MimeType::yaml)
							<< endl;
				}

				// Test agent interface
				{
					class YamlRequest : public Request {
					public:
						YamlRequest(const char *path) : Request{path} { 
						}

						MimeType mimetype() const noexcept override {
							return MimeType::yaml;
						}

					};

					// Build root agent to initialize agent interface.
					auto root = Abstract::Agent::RootFactory();
					root->push_back(make_shared<Agent<int>>("intvalue"));
					Abstract::Agent::Controller::getInstance().set(root);

					// Get root agent properties
					for(const char *path : { "/agent", "/agent/intvalue", "/api/agent", "/api/agent/intvalue" }) {

						YamlRequest request{path};

						auto intf = Interface::find(request);
						if(!intf) {
							throw runtime_error(String{"Cant find interface for '",request.path(),"'"});
						}

						Schema schema;
						if(!intf->output_schema(request.path(),schema)) {
							throw runtime_error("Interface doesnt provides an output-schema");
						}

						stream << "Processing" << (request.apicall() ? " API " : " ") 
							<< "request for '" << request.path() << "' using interface '" 
							<< intf->name() << "':" << endl;
						if(!intf->process(request,stream)) {
							throw runtime_error(String{"Interface was unable to process '",request.path(),"'"});
						}
						stream << endl;

					}

				}

				return "Basic agent tests passed";
			}
		}
	);

 }

 #endif // DEBUG

 