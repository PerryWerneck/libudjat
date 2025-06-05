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
 #include <udjat/defs.h>
 #include <udjat/tools/commandlineparser.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	static void extract(int dst, int &argc, char **argv) noexcept {
		for(int src = dst+1; src < argc; src++) {
			argv[dst++] = argv[src];                
		}
		argc--;
	}
		
	bool CommandLineParser::has_argument(int &argc, char **argv, char shortname, const char *longname, bool remove) noexcept{

		if(argc < 2) {
			debug("No arguments to parse searching for '",longname,"' (argc=",argc,")");
			return false;
		}

		debug("Argc=",argc);
		size_t szlong = 0;
		if(longname && *longname) {
			szlong = strlen(longname);
		}

		for(int ix = 1; ix < argc; ix++) {

			debug("ix=",ix," arg='",argv[ix],"'");

			if(argv[ix][0] != '-') {
				continue;
			}

			if(shortname && argv[ix][1] == shortname && !argv[ix][2]) {
				debug("Found short arg '",argv[ix],"'");
				if(remove) {
					extract(ix,argc,argv);
				}
				return true;
			}

			if(!szlong) {
				continue;
			}

			if(argv[ix][1] == '-' && !strcmp(longname,(argv[ix]+2)) && argv[ix][szlong+2] == 0) {
				debug("Found long arg '",(argv[ix]+2),"'");
				if(remove) {
					extract(ix,argc,argv);
				}
				return true;			
			}

		}

		return false;
	}

	bool CommandLineParser::get_argument(int &argc, char **argv, char shortname, const char *longname, std::string &value, bool remove) noexcept {

		value.clear();

		if(argc < 2) {
			debug("No arguments to parse searching for '",longname,"' (argc=",argc,")");
			return false;
		}

		debug("Argc=",argc);
		size_t szlong = 0;
		if(longname && *longname) {
			szlong = strlen(longname);
		}

		for(int ix = 1; ix < argc; ix++) {

			debug("ix=",ix," arg='",argv[ix],"'");

			if(argv[ix][0] != '-' || argv[ix][1] == 0) {
				continue;
			}

			if(argv[ix][1] == '-' && argv[ix][2] && szlong && !strncasecmp(longname,(argv[ix]+2),szlong) && argv[ix][szlong+2] == '=') {
				value.assign(argv[ix]+szlong+3);
				if(remove) {
					extract(ix,argc,argv);
				}
				debug("Found long arg '",longname,"'='",value,"'");
				return true;			
			} else if(szlong && !strncasecmp(longname,(argv[ix]+1),szlong) && argv[ix][szlong+1] == '=') {
				value.assign(argv[ix]+szlong+2);
				if(remove) {
					extract(ix,argc,argv);
				}
				debug("Found long arg '",longname,"'='",value,"'");
				return true;			
			}

			if(!shortname || argv[ix][1] != shortname) {
				continue;
			}

			debug("Found short arg '",argv[ix],"'");

			if(argv[ix][2] != 0) {
				value.assign(argv[ix]+2);
				if(remove) {
					extract(ix,argc,argv);
				}
				debug("Found short arg '",shortname,"'='",value,"'");
				return true;
			}

			if(ix == (argc-1) || argv[ix+1][0] == '-') {
				debug("Ignoring short arg '",argv[ix],"'");
				continue;
			}

			value = argv[ix+1];
			if(remove) {
				extract(ix,argc,argv);
				extract(ix,argc,argv);
			}

			debug("Found short arg '",shortname,"'='",value,"'");
			return true;

		}

		return false;

	}

	/// @brief Common help message for all applications.
	static void apphelp(size_t width) {
		static const CommandLineParser::Argument values[] = {
			{ 'h', "help", _("Show this help message") },
		};

		cout << _("Application options:\n");
		for(const auto &value : values) {
			value.print(cout,width);
			cout << "\n";
		};

	}

	void Application::help(size_t width) const noexcept {

		cout 	<< _("Usage:") << "\n  " << argv[0]
				<< " " << _("[OPTION..]") << "\n\n";

		apphelp(width);

		static const CommandLineParser::Argument values[] = {
			{ 'T', "timer=seconds", _("Quit after \"seconds\"") },
		};

		for(const auto &value : values) {
			value.print(cout,width);
			cout << "\n";
		};

	}

	/*
	static void setProperty(const char *name, const char *value) {

		debug("Property: '",name,"'('",(value ? value : "NULL"),"')");

#ifdef _WIN32
		if(!SetEnvironmentVariable(name,value)) {
			throw Win32::Exception(_("Unable to set environment variable"));
		}
#else
		if(setenv(name, value, 1)) {
			throw std::system_error(errno,std::system_category(),_("Unable to set environment variable"));
		}
#endif // _WIN32

	}
	*/

	void CommandLineParser::setup(int &argc, char **argv, bool dbg) noexcept {

		// Setup logger
		Logger::setup(argc,argv,dbg);
		Logger::redirect();	

		if(dbg) {
			Logger::console(true);
		}

		// Setup properties
		{
			int ix = 1;
			while(ix < argc) {

				const char *eq = strchr(argv[ix],'=');

				if(argv[ix][0] == '-' || !eq) {
					ix++;
					continue;
				}

				string name{argv[ix],(size_t) (eq-argv[ix])};
				eq++;

				String value{eq};
				value.expand();

				extract(ix,argc,argv);

#ifdef _WIN32
				if(!SetEnvironmentVariable(name,value)) {
					Logger::String{"Unable to set property '",name.c_str(),"' to '",value.c_str(),"'"}.error();
				} else {
					Logger::String{"Property '",name.c_str(),"' set to '",value.c_str(),"'"}.trace();
				}
#else
				if(setenv(name.c_str(), value.c_str(), 1)) {
					Logger::String{"Unable to set property '",name.c_str(),"' to '",value.c_str(),"'"}.error();
				} else {
					Logger::String{"Property '",name.c_str(),"' set to '",value.c_str(),"'"}.trace();
				}
#endif // _WIN32
			}
		}

	}

	bool CommandLineParser::options(int &argc, char **argv, const CommandLineParser::Argument *options, bool dbg, size_t width) noexcept {

		if(!has_argument(argc,argv,'h',"help",true)) {

			CommandLineParser::setup(argc,argv,dbg);
			return false;
		}

		cout	
			<< _("Usage:") << "\n  " << argv[0]
			<< " " << _("[OPTION..]") << "\n\n";

		apphelp(width);
		if(options) {
			for(const Argument *option = options; option->description; option++) {
				option->print(cout,width);
				cout << "\n" ;
			};
		}

		cout << "\n";
		Logger::help(width);

		cout << "\n\n";

		return true;

	}

	std::ostream & Udjat::CommandLineParser::Argument::print(std::ostream &out, size_t width) const {
		out << "  -" << shortname << ", --";			
		string text{longname};
		text.resize(width,' ');
		out << text << " " << description;
		return out;
	}

 }


