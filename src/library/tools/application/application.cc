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
 #include <udjat/tools/intl.h>

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

	Dialog::Status & Application::state(const Level, const char *message) noexcept {
		Logger::String{message}.write((Logger::Level) (Logger::Debug+1),Name().c_str());
		return *this;
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

	bool Application::has_argument(char shortname, const char *longname) noexcept{
		return CommandLineParser::has_argument(argc,argv,shortname,longname,true);
	}

	bool Application::get_argument(char shortname, const char *longname, std::string &value) noexcept{
		return CommandLineParser::get_argument(argc,argv,shortname,longname,value,true);
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

 }

