/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/application.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/module/abstract.h>
 #include <string>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <vector>
 #include <libgen.h>
 #include <filesystem>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	// Check for help
	if(Application::pop(argc,argv,'h',"help")) {
		static const string help_text = 
			"\t-m, --module <module>        Load module\n"
			"\t-c, --config <file or path>  XML definitions\n"
			"\t-S, --service                Run as system service\n"
			"\t-h, --help                   Show this help message\n";
		
		cout << "Usage: " << argv[0] << " [options]" << endl << endl << help_text;
		return 0;
	}

	Config::allow_user_homedir(true);
	Logger::verbosity(9);
	Logger::redirect();
	Logger::console(true);

	// Test mode
	enum Mode {
		Application,	// Run as application
		Service,		// Run as service
	} mode = Application;

	// Configuration file (or path)
	string config_file = "./test.xml";

	// Loaded modules
	vector<Module *> modules;

	// Check arguments
	{
		string argvalue;

		if(Application::pop(argc,argv,'m',"module",argvalue)) {
			Logger::String{"Loading module '" + argvalue + "'"}.info();
			modules.push_back(Module::factory(argvalue.c_str()));
			if(modules.back() == nullptr) {
				Logger::String{"Module '" + argvalue + "' not found"}.error();
				return -1;
			}
			modules.back()->test_mode();
		}
	
		if(Application::pop(argc,argv,'c',"config",argvalue)) {
			config_file = argvalue;
		}

		if(Application::pop(argc,argv,'S',"service",argvalue)) {
			mode = Service;
		}

	}

	Logger::String{"Loading definitions from '" + config_file + "'"}.info();

	{
		string testmodule{".build/testmodule" LIBEXT};

		if(access(testmodule.c_str(),R_OK) == 0) {
			Logger::String{"Loading test module from '" + testmodule + "'"}.info();
			modules.push_back(Module::factory(testmodule.c_str()));
			if(modules.back() == nullptr) {
				Logger::String{"Module '" + testmodule + "' not found"}.error();
				return -1;
			}
			modules.back()->test_mode();
		}

	}

	switch(mode) {
	case Application:
		{
			// Run as application
			Udjat::Application app{argc,argv};
			int rc = app.run(config_file.c_str());
			if(rc != 0) {
				Logger::String{"Application failed with error '",strerror(rc),"' (",rc,")"}.error();
				return rc;
			}	
		}
		break;

	case Service:
		{
			// Run as service
			Udjat::SystemService srvc{argc,argv};
			int rc = srvc.run(config_file.c_str());
			if(rc != 0) {
				Logger::String{"Service failed with error '",strerror(rc),"' (",rc,")"}.error();
				return rc;
			}	
		}
		break;
	}

	return 0;

 }
