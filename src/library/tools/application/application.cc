/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/logger.h>
 #include <udjat/agent.h>
 #include <udjat/module/abstract.h>
 #include <iostream>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <udjat/tools/quark.h>
 #include <udjat/agent/abstract.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	UDJAT_API const char * revision() {
		return STRINGIZE_VALUE_OF(BUILD_DATE);
	}

	Application::~Application() {
		if(timer) {
			delete timer;
		}
		Udjat::Module::unload();
	}

	std::shared_ptr<Abstract::Agent> Application::RootFactory() const {
		return Abstract::Agent::RootFactory();
	}

	std::ostream & Application::info() {
		return cout << Application::Name::getInstance() << "\t";
	}

	std::ostream & Application::warning() {
		return clog << Application::Name::getInstance() << "\t";
	}

	std::ostream & Application::error() {
		return cerr << Application::Name::getInstance() << "\t";
	}

	std::ostream & Application::trace() {
		return Logger::trace() << Application::Name::getInstance() << "\t";
	}

	Application::DataFile::DataFile(const char *name, bool system) {
		if(name[0] == '/' || (name[0] == '.' && name[1] == '/') || name[0] == '\\' || (name[0] == '.' && name[1] == '\\') || name[1] == ':' ) {
			assign(name);
		} else {
			if(system) {
				assign(SystemDataDir());
			} else {
				assign(DataDir());
			}
			append(name);
		}
	}

	Application::DataFile::DataFile(const XML::Node &node, const char *attrname, bool system) : DataFile(nullptr,node,attrname,system) {
	}

	Application::DataFile::DataFile(const char *type, const XML::Node &node, const char *attrname, bool system) {

		const char *name = node.attribute(attrname).as_string("");

		if(!name[0]) {
			throw runtime_error(Logger::String("Required attribute '",attrname,"' is missing"));
		}

		if(name[0] == '/' || (name[0] == '.' && name[1] == '/') || name[0] == '\\' || (name[0] == '.' && name[1] == '\\') || name[1] == ':' ) {
			assign(name);
			return;
		}

		if(node.attribute("system-data-dir").as_bool(system)) {
			assign(SystemDataDir());
		} else {
			assign(DataDir());
		}

		if(type) {
			append(type);
			File::Path::mkdir(c_str());
			append("/");
		}

		append(name);

	}

	bool Application::DataFile::available() const noexcept {
		return access(c_str(),R_OK) == 0;
	}

	Application::LogDir & Application::LogDir::getInstance() {
		static LogDir instance;
		return instance;
	}

	static void extract(int dst, int &argc, char **argv) {

		for(int src = dst+1; src < argc; src++) {
			argv[dst++] = argv[src];		
		}
		argc--;

	}

	bool Application::pop(int argc, char **argv, char shortname, const char *longname) {

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
				extract(ix,argc,argv);
				return true;
			
			}

			if(!szlong) {
				continue;
			}

			if(argv[ix][1] == '-' && !strcmp(longname,(argv[ix]+2)) && argv[ix][szlong+2] == 0) {
				debug("Found long arg '",(argv[ix]+2),"'");
				extract(ix,argc,argv);
				return true;			
			}

		}

		return false;
	}

	bool Application::pop(int argc, char **argv, char shortname, const char *longname, std::string &value) {

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
				debug("ix=",ix," argc=",argc);
				if(ix == (argc-1) || argv[ix+1][0] == '-') {
					debug("Ignoring short arg '",argv[ix],"'");
					continue;
				}
				value = argv[ix+1];
				extract(ix,argc,argv);
				debug("--------------> ",ix," ",argv[ix]);
				extract(ix,argc,argv);
				debug("--------------> ",ix," ",argv[ix]);
				debug("Found short arg '",shortname,"'='",value,"'");
				return true;
			
			}

			if(!szlong) {
				continue;
			}

			if(argv[ix][1] == '-' && !strncmp(longname,(argv[ix]+2),szlong) && argv[ix][szlong+2] == '=') {
				value=argv[ix]+szlong+3;
				extract(ix,argc,argv);
				debug("Found long arg '",longname,"'='",value,"'");
				return true;			
			}

		}

		return false;

	}

	/*
	const char * Application::getProperty(const char *name, const char *def) const noexcept {

		size_t szname = strlen(name);

		for(int ix = 1; ix < args.count; ix++) {

			const char *ptr = args.value[ix];
			while(*ptr && *ptr == '-') {
				ptr++;
			}

			if(!*ptr || strncasecmp(ptr,name,szname)) {
				continue;
			}

			if(ptr[szname]) {
				if(ptr[szname] != '=') {
					continue;
				}

				return ptr+szname+1;
			}

			return "";

		}

		const char *env = getenv(name);
		if(env) {
			return env;
		}

		return def;

	}
	*/

 }

