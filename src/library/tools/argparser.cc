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
			[this](const char *argument) {
				show_help();
				return true;
			}
		);
	}

	bool ArgumentParser::parse(int &argc, const char **argv, const Argument *arguments) {

		ArgumentParser parser;

		for(const Argument *arg = arguments; *arg; arg++) {
			parser.add_application_argument(*arg);
		}

		return parser.parse(argc,argv);

	}

	bool ArgumentParser::parse(int &argc, const char **argv) const {

		for(int ix = 0; ix < argc; ix++) {

			const char *arg = argv[ix];

			if(*arg != '-') {
				continue;
			}

			arg++;

			if(*arg == '-') {

				// Parse long argument.
				arg++;

				if(parse_long(arg,argv+ix+1)) {
					return true;
				}

				continue;

			}

			// Parse short arguments
			while(*arg) {
				if(parse_short(arg,argv+ix+1)) {
					return true;
				}
				arg++;
			}

		}

		return true;
	}

	void ArgumentParser::add_application_argument(const ArgumentParser::Argument &argument) {
		groups.front().push_back(argument);
	}

	void ArgumentParser::add_application_argument(const char shortname, const char *longname, const char *description, const std::function<bool(const char *argument)> &call) {
		groups.front().emplace_back(shortname,longname,description,call);
	}

	/*
	void ArgumentParser::append(const Argument &argument) {
		groups.back().push_back(argument);
	}
	*/

	ArgumentParser::Group & ArgumentParser::add_group(const char *text) {
		groups.emplace_back(text);
		return groups.back();
	}

	bool ArgumentParser::show_help() const {

		debug("Running ",__FUNCTION__);

		size_t len = 0;
		// bool decorated = Logger::decorated();

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

		for(const auto &group : groups) {
			cout << group.c_str() << ":\n";	
			for(const auto &arg : group) {

				cout << "  ";

				if(arg.shortname) {
					cout << '-' << arg.shortname;
				} else {
					cout << "  ";
				}
				cout << " ";

				{
					char buffer[len+1];
					memset(buffer,' ',len);

					if(arg.longname && *arg.longname) {
						cout << "--";
						strncpy(buffer,arg.longname,strlen(arg.longname));
					} else {
						cout << "  ";
					}

					buffer[len] = 0;
					cout << buffer;

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

	bool ArgumentParser::parse_short(const char *argument, const char **argv) const {

		debug(__FUNCTION__,"(",argument,")");

		// Get optional parameters.
		const char *value = nullptr;
		
		if(argv && *argv) {
			value = *argv;
			if(!value || value[0] == '-') {
				value = nullptr;
			}
		}

		// Parse 'argument'
		for(const auto &group : groups) {
			for(const auto &arg : group) {
				if(arg == *argument) {
					return arg.exec(value);
				}
			}
		}

		char str[] = {argument[0],0};
		throw runtime_error(Logger::Message{_("Invalid option: -{}"),str});

	}

	bool ArgumentParser::parse_long(const char *argument, const char **argv) const {

		debug(__FUNCTION__,"(",argument,")");

		// Get optional parameters.
		const char *value = nullptr;
		
		if(argv && *argv) {
			value = *argv;
			if(!value || value[0] == '-') {
				value = nullptr;
			}
		}

		// Parse 'argument'
		for(const auto &group : groups) {
			for(const auto &arg : group) {
				if(arg == argument) {
					return arg.exec(value);
				}
			}
		}

		throw runtime_error(Logger::Message{_("Invalid option: --{}"),argument});

	}

 }

