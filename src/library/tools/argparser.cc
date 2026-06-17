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

 using namespace std;

 namespace Udjat {

	ArgumentParser::ArgumentParser() {

		// The first group is allways the application options
		groups.emplace_back(_("Application options"));

	}

	ArgumentParser::ArgumentParser(const char *str) : ArgumentParser{} {
		groups.emplace_back(str);
	}

	ArgumentParser::~ArgumentParser() {

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
				if(parse_short(&arg,argv+ix+1)) {
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

	void ArgumentParser::append(const Argument &argument) {
		groups.back().push_back(argument);
	}

	ArgumentParser::Group & ArgumentParser::add_group(const char *text) {
		groups.emplace_back(text);
		return groups.back();
	}

	bool ArgumentParser::show_help() const {
		const char *message = _("This help message");
		size_t len = strlen(message);

		// Get option width.
		for(const auto &group : groups) {
			for(const auto &arg : group) {
				if(arg.longname) {
					len = std::max(len,strlen(arg.longname));
				}
			}
		}

		// Show options.
		for(const auto &group : groups) {
			cout << group.c_str() << "\n";	
			for(const auto &arg : group) {

				char buffer[len+7];
				memset(buffer,' ',len+7);

				if(arg.shortname) {
					buffer[2] = '-';
					buffer[3] = arg.shortname;
				}

				if(arg.longname && *arg.longname) {
					buffer[4] = '-';
					buffer[5] = '-';
					strncpy(buffer+6,arg.longname,strlen(arg.longname));
				}
				
				buffer[len+7] = 0;
				cout << buffer;

				if(arg.help && *arg.help) {
					cout << arg.help;
				}
				cout << "\n";
			}
		}

		return true; // End application
	}

	bool ArgumentParser::parse_short(const char **argument, const char **argv) const {

		if(**argument == 'h') {
			return show_help();
		}

		// Get optional parameters.
		const char *arg = *argv;
		if(!arg || arg[0] == '-') {
			arg = nullptr;
		}

		// TODO: Parse 'argument'

		return false;
	}

	bool ArgumentParser::parse_long(const char *argument, const char **argv) const {

		if(!(strcmp(argument,"help") && strcmp(argument,_("help")))) {
			return show_help();
		}

		// Get optional parameters.
		const char *arg = *argv;
		if(!arg || arg[0] == '-') {
			arg = nullptr;
		}

		// TODO: Parse 'argument'

		return false;
	}

 }

