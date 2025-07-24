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
 #include <signal.h>
 #include <cstdarg>

 #ifdef HAVE_INIPARSER

 #include <iniparser.h>

 #define LOG_DOMAIN "config"
 #include <udjat/tools/logger.h>

 #include <private/configuration.h>

 namespace Udjat {

	static int error_callback(const char *format, ...) {
		va_list args;
		va_start(args, format);
		char *message = NULL;
		vasprintf(&message, format, args);
		va_end(args);
		if(message) {
			Logger::String{message}.error("iniparser");
			free(message);
		}
		return 0; // Return 0 to continue processing.
	}

	static inline Udjat::String key(const char *group, const char *key) {
		return Udjat::String{group,":",key};
	}

	std::recursive_mutex Config::Controller::guard;

	Config::Controller::Controller() {

		iniparser_set_error_callback(error_callback);

		ini = iniparser_load(String{"/etc/",program_invocation_short_name,".conf"}.c_str());
		if(!ini) {
			Logger::String{"Cant load /etc/",program_invocation_short_name,".conf, using default configuration"}.warning("iniparser");
		} else if(Logger::enabled(Logger::Trace)) {
			Logger::String{"Got configuration from /etc/",program_invocation_short_name,".conf'"}.trace("iniparser");
		}
	}

	Config::Controller::~Controller() {
		if(ini) {
			iniparser_freedict(ini);
			ini = nullptr;
		}
		iniparser_set_error_callback(NULL);
	}

	void Config::Controller::open() {
	}

	void Config::Controller::close() {
	}

	void Config::Controller::reload() {
	}

	bool Config::Controller::hasGroup(const char *group) {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return false;
		}
		return (iniparser_getsecnkeys(ini,(char *)group) != 0);
	}

	bool Config::Controller::hasKey(const char *group, const char *key) {
		std::lock_guard<std::recursive_mutex> lock(guard);
		return true;
	}

	int32_t Config::Controller::get(const char *group, const char *name, const int32_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return def;
		}
		return iniparser_getint(ini,key(group,name).c_str(),def);
	}

	int64_t Config::Controller::get(const char *group, const char *name, const int64_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return def;
		}
		return iniparser_getint(ini,key(group,name).c_str(),def);
	}

	uint32_t Config::Controller::get(const char *group, const char *name, const uint32_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return def;
		}
		return iniparser_getint(ini,key(group,name).c_str(),def);
	}

	uint64_t Config::Controller::get(const char *group, const char *name, const uint64_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return def;
		}
		return (unsigned int) iniparser_getint(ini,key(group,name).c_str(),def);
	}

	float Config::Controller::get(const char *group, const char *name, const float def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return def;
		}
		return iniparser_getdouble(ini,key(group,name).c_str(),def);
	}

	double Config::Controller::get(const char *group, const char *name, const double def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return def;
		}
		return iniparser_getdouble(ini,key(group,name).c_str(),def);
	}

	bool Config::Controller::get(const char *group, const char *name, const bool def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(!ini) {
			return def;
		}
		return iniparser_getboolean(ini,key(group,name).c_str(),def);
	}

	Udjat::String Config::Controller::get_string(const char *group, const char *name, const char *def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);
		debug("group='",group,"' name='",name,"'");
		if(!ini) {
			return Udjat::String{def};
		}
		String str{iniparser_getstring(ini,key(group,name).c_str(),(char *) def)};

		debug(group,":",name,"='",str.c_str(),"'");
		return str;
	}

	bool Config::Controller::for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {

		if(!ini) {
			return false;
		}

		std::lock_guard<std::recursive_mutex> lock(guard);
		size_t items = iniparser_getsecnkeys(ini,(char *) group);
		if(!items) {
			debug("No items in group '",group,"'");
			return false;
		}

		char **keys = new char *[items];

		if(!iniparser_getseckeys(ini, (char *) group, (const char **) keys)) {
			debug("Failed to get keys for group '",group,"'");
			delete[] keys;
			return false;
		}

		bool found = false;
		for(size_t ix = 0;ix < items && !found; ix++) {
			const char *value = iniparser_getstring(ini,keys[ix],"");
			if(value) {
				try {
					const char *ptr = strchr(keys[ix],':');
					found = call((ptr ? (ptr+1) : keys[ix]),value);
				} catch(const std::exception &e) {
					debug("Error processing key='",keys[ix],"': ",e.what());
					break; // Stop iterating.
				}
			}
		}

		delete[] keys;
		return found;

	}

 }

 #endif // HAVE_INIPARSER



