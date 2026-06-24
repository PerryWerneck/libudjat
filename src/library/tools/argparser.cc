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
 #include <udjat/defs.h>
 #include <string>
 #include <udjat/tools/argumentparser.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/ui/console.h>
 #include <ctype.h>
 #include <iostream>
 #include <fstream>

 using namespace std;

 namespace Udjat {

	ArgumentParser::ArgumentParser() {

		// The first group is allways the application options
		groups.emplace_back(_("Application options"));
		append_help();

	}

	ArgumentParser::ArgumentParser(const char *str) {
		groups.emplace_back(str);
		append_help();
	}

	ArgumentParser::ArgumentParser(const Argument &arg) : ArgumentParser{} {
		groups.front().push_back(arg);	
	}

	ArgumentParser::~ArgumentParser() {

	}

	void ArgumentParser::append_help() {
		groups.front().emplace_back(
			'h', 
			"help", 
			_("Show this help message"),
			[this](const char *argument, bool) {
				show_help();
				return true;
			}
		);
	}

	bool ArgumentParser::parse(int argc, char **argv, const Argument *arguments) {

		ArgumentParser parser;

		for(const Argument *arg = arguments; *arg; arg++) {
			parser.add_application_argument(*arg);
		}

		return parser.parse(argc,argv);

	}

	bool ArgumentParser::parse(int argc, char **argv) const {

		for(int ix = 0; ix < argc; ix++) {

			const char *arg = argv[ix];

			if(*arg != '-') {
				continue;
			}

			arg++;

			// Check for long argument
			if(*arg == '-') {

				// Parse long argument.
				arg++;

				const char *ptr = strchr(arg,'=');
				if(ptr) {
					ptr++;
				}

				if(parse_long(arg,ptr,'L')) {
					return true;
				}

				continue;

			}

			// Check for short argument
			const char *value = nullptr;
			if(ix < (argc-1) && argv[ix+1][0] != '-') {
				value = argv[ix+1];
			}

			char last = 0;
			char index = '0';
			while(*arg) {

				if(isdigit(arg[1])) {

					// Repeat 'arg[1]' times.
					for(int ix='0';ix < arg[1];ix++) {
						if(parse_short(arg,value,ix)) {
							return true;
						}
					}
					arg++;

				} else if(arg[0] == last) {

					// It's repeating argument
					if(parse_short(arg,value,index++)) {
						return true;
					}

				} else if(arg[0] == arg[1]) {
					
					// It's the first one of a repetittion
					last = arg[1];
					index = '0';
					if(parse_short(arg,value,index++)) {
						return true;
					}
				} else {

					// It's not repeating
					if(parse_short(arg,value,'S')) {
						return true;
					}

				}
				arg++;
				
			}

		}

		/// Complete without errors
		return false;
	}

	void ArgumentParser::add_application_argument(const ArgumentParser::Argument &argument) {
		groups.front().push_back(argument);
	}

	void ArgumentParser::add_application_argument(const char shortname, const char *longname, const char *description, const std::function<bool(const char *argument, const char mode)> &call) {
		groups.front().emplace_back(shortname,longname,description,call);
	}

	ArgumentParser::Group & ArgumentParser::add_group(const char *text) {
		groups.emplace_back(text);
		return groups.back();
	}

	bool ArgumentParser::show_help() const {

		debug("Running ",__FUNCTION__);

		size_t len = 0;
		bool decorated = Console::decorated();

		// Get option width.
		for(const auto &group : groups) {
			for(const auto &arg : group) {
				if(arg.longname) {
					debug("longname='",arg.longname,"' len=",strlen(arg.longname));
					len = std::max(len,strlen(arg.longname));
				}
			}
		}

		debug("---> Max long option length is ",len);

		cout << _("Usage:") << "\n  ";

#ifdef _WIN32

		// TODO: Get path of windows app.
		cout << Application::Name() << " ";

#else
		// Linux. Get my name from /proc/cmdline
		{
			std::ifstream file("/proc/self/cmdline");

			if(file.is_open()) {
				string line;
				getline(file,line,'\0');
				file.close();
				cout << line;

			} else {
				cout << "application";
			}

			cout << " ";
			
		}

#endif

		if(decorated) {
			cout << "\x1B[2m";
		}

		cout << _("[OPTIONS]");
		
		if(decorated) {
			cout << "\x1B[22m";
		}

		cout << "\n\n";

		for(const auto &group : groups) {
			if(decorated) {

				cout << "\x1B[1m" << group.c_str() << ":" << "\x1B[22m" << "\n";	

			} else {
	
				cout << group.c_str() << ":\n";	

			}
			for(const auto &arg : group) {

				cout << "  ";

				if(arg.shortname) {
					cout << '-' << arg.shortname;
				} else {
					cout << "  ";
				}
				cout << " ";

				{
					size_t sz = 0;

					if(arg.longname && *arg.longname) {
						cout << "--" << arg.longname;
						sz = strlen(arg.longname);
					} else {
						cout << "  ";
					}

					while(sz++ < len) {
						cout << " ";
					}

				}

				if(arg.help && *arg.help) {
					cout << " " << arg.help;
				}
				cout << "\n";
			}
			cout << "\n";
		}

		return true; // End application
	}

	bool ArgumentParser::parse_short(const char *argument, const char *value, const char mode) const {

		debug(__FUNCTION__,"(",argument,")");

		// Parse 'argument'
		for(const auto &group : groups) {
			for(const auto &arg : group) {
				if(arg == *argument) {
					return arg.exec(value,mode);
				}
			}
		}

		char str[] = {argument[0],0};
		throw runtime_error(Logger::Message{_("Invalid option: -{}"),str});

	}

	bool ArgumentParser::parse_long(const char *argument, const char *value, const char mode) const {

		debug(__FUNCTION__,"(",argument,")");

		// Parse 'argument'
		for(const auto &group : groups) {
			for(const auto &arg : group) {
				if(arg == argument) {
					return arg.exec(value,mode);
				}
			}
		}

		throw runtime_error(Logger::Message{_("Invalid option: --{}"),argument});

	}

	void ArgumentParser::call(const char *option) {
		for(const auto &group : groups) {
			for(const auto &arg : group) {
				if(arg == option) {
					arg.exec();
					return;
				}
			}
		}
	}


 }

