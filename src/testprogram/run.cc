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
 #include <iostream>
 #include <string.h>
 #include <udjat/tools/configuration.h>
 
 using namespace std;

 namespace Udjat {

	int Testing::run(int argc, char **argv, const Udjat::ModuleInfo &info, const char *xml) {

		return run(argc,argv,info,[](Udjat::Application &){

		},xml);

		/*
		Config::allow_user_homedir(true);

		Logger::redirect();
		Logger::verbosity(Config::Value<unsigned int>{"test-mode","verbose",9}.get());
		Logger::console(Config::Value<bool>{"test-mode","console",true}.get());
		
		Config::Value<string> setup{"test-mode","xml_path",xml};

		if(argc == 1) {

			int rc = -1;

			if(strcasecmp(Config::Value<string>{"test-mode","run-as","application"}.c_str(),"application")) {
				rc = Testing::Service{info}.run(1,argv,setup.c_str());
				Logger::String{"Service exits with rc=",rc}.info("test");
			} else {
				rc = Testing::Application{info}.run(1,argv,setup.c_str());
				Logger::String{"Application exits with rc=",rc}.info("test");
			}

			return rc;
		}

		for(int opt = 1; opt < argc; opt++) {

			const char *ptr = argv[opt];
			while(*ptr && *ptr == '-') {
				ptr++;
			}

			if(!strcasecmp(ptr,"application")) {

				auto rc = Testing::Application{info}.run(1,argv,setup);
				Logger::String{"Application exits with rc=",rc}.info("test");
				return rc;

			} else if(!strcasecmp(ptr,"service")) {

				auto rc = Testing::Service{info}.run(1,argv,setup);
				Logger::String{"Service exits with rc=",rc}.info("test");
				return rc;

			}

		}

		return -1;
		*/

	}

	int Testing::run(int argc, char **argv, const Udjat::ModuleInfo &info, const std::function<void(Udjat::Application &app)> &initialize, const char *xml) {

		int rc = -1;

		Logger::redirect();
		Logger::verbosity(9);
		Logger::console(true);

		Config::Value<string> setup{"test-mode","xml_path",xml};

		if(argc == 1) {
			rc = Testing::Application{info,initialize}.run(argc,argv,setup.c_str());
			Logger::String{"Application exits with rc=",rc}.info("test");
			return rc;
		}

		for(int opt = 1; opt < argc; opt++) {

			const char *ptr = argv[opt];
			while(*ptr && *ptr == '-') {
				ptr++;
			}

			if(!strcasecmp(ptr,"application")) {

				rc = Testing::Application{info,initialize}.run(1,argv,setup.c_str());
				Logger::String{"Application exits with rc=",rc}.info("test");
				return rc;

			} else if(!strcasecmp(ptr,"service")) {

				rc = Testing::Service{info,initialize}.run(1,argv,setup.c_str());
				Logger::String{"Service exits with rc=",rc}.info("test");
				return rc;

			}

		}

		if(strcasecmp(Config::Value<string>{"test-mode","run-as","application"}.c_str(),"application")) {
			rc = Testing::Service{info}.run(argc,argv,setup.c_str());
			Logger::String{"Service exits with rc=",rc}.info("test");
		} else {
			rc = Testing::Application{info}.run(argc,argv,setup.c_str());
			Logger::String{"Application exits with rc=",rc}.info("test");
		}

		return rc;
		
	}

 }

