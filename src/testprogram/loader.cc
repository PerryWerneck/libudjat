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
 #include <udjat/tools/commandlineparser.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/module/abstract.h>
 #include <string>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <vector>
 #include <libgen.h>

 #include <private/randomfactory.h>

 #ifdef HAVE_FILESYSTEM_H
	 #include <filesystem>
 #endif

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	Logger::verbosity(9);
	Logger::console(true);
	Config::allow_user_homedir(true);

	// Check for help
	static const Udjat::CommandLineParser::Argument options[] = {
		{ 'h', "help",				"Show this help message"		},
		{ 'A', "application", 		"Run as application"			},
		{ 'S', "service", 			"Run as system service"			},
		{ 'm', "module=<module>",	"Load module by name or path"	},
		{ 'c', "config=<path>",		"Load XML configuration from file or directory" },
	};

	if(CommandLineParser::has_argument(argc,argv,'h',"help",true)) {

		cout	<< "Usage:" << "\n  " << argv[0]
				<< " " << "[OPTION..]" << "\n\n";

		cout << "Common options:\n";
		for(const auto &option : options) {
			option.print(cout);
			cout << "\n";
		};
		Application::show_command_line_help();

		cout << "\nService options:\n";
		SystemService::show_command_line_help();

		cout << "\n";
		Logger::help();

		return 0;
	}

//	if(Udjat::CommandLineParser::options(argc,argv,options)) {
//		return 0;
//	}

	Logger::redirect();

	// Configuration file (or path)
	string config_file = "./test.xml";

	// Loaded modules
	vector<Module *> modules;

	// Check arguments
	{
		string argvalue;

		if(CommandLineParser::get_argument(argc,argv,'m',"module",argvalue)) {
			Logger::String{"Loading module '" + argvalue + "'"}.info();
			modules.push_back(Module::factory(argvalue.c_str()));
			if(modules.back() == nullptr) {
				Logger::String{"Module '" + argvalue + "' not found"}.error();
				return -1;
			}
			modules.back()->test_mode();
		}
	
		if(CommandLineParser::get_argument(argc,argv,'c',"config",argvalue)) {
			config_file = argvalue;
		}

	}

	Logger::String{"Loading definitions from '" + config_file + "'"}.info();

	RandomFactory rfactory;
	string testmodule{".build/testmodule" LIBEXT};
	
	if(CommandLineParser::has_argument(argc,argv,'S',"service")) {

		// Run as service
		int rc = Udjat::SystemService{argc,argv}.run(config_file.c_str());
		if(rc != 0) {
			Logger::String{"Service failed with error '",strerror(rc),"' (",rc,")"}.error();
			return rc;
		}	

	} else if(CommandLineParser::has_argument(argc,argv,'A',"application")) {

		// Run as application
		int rc = Udjat::Application{argc,argv}.run(config_file.c_str());
		if(rc != 0) {
			Logger::String{"Application failed with error '",strerror(rc),"' (",rc,")"}.error();
			return rc;
		}	

	} else if(access(testmodule.c_str(),R_OK) == 0) {
		Logger::String{"Loading test module from '" + testmodule + "'"}.info();
		modules.push_back(Module::factory(testmodule.c_str()));
		if(modules.back() == nullptr) {
			Logger::String{"Module '" + testmodule + "' not found"}.error();
			return -1;
		}
		modules.back()->test_mode();

	} else {

		Logger::String{"No service or application mode, and no filesystem support to load test module"}.warning();
		Logger::String{"Run with '--service' or '--application' option to run as service or application"}.warning();
		return -1;			

	}

	return 0;

 }
