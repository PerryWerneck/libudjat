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
 #include <stdexcept>

 using namespace std;

 #ifdef HAVE_ECONF

 // https://github.com/openSUSE/libeconf
 extern "C" {
	#include <libeconf.h>
 }

 #define LOG_DOMAIN "config"
 #include <udjat/tools/logger.h>

 #include <private/configuration.h>

 namespace Udjat {

#ifdef ECONF_HAVE_READ_CONFIG_WITH_CALLBACK
	static bool checkFile(const char *filename, const void *) {
		Logger::String{"Loading '",filename,"'"}.trace("econf");
		return true;
	}
#endif

	static void handle_reload(int sig) noexcept {

		Logger::String{"Reloading configuration by signal '",(const char *) strsignal(sig),"'"}.write(Logger::Trace);

		try {

			Config::Controller::getInstance().reload();

		} catch(const std::exception &e) {

			Logger::String{"Error '", e.what(), "' reloading configuration"}.error();

		} catch(...) {

			Logger::String{"Unexpected error reloading configuration"}.error();

		}

	}

	std::recursive_mutex Config::Controller::guard;

	Config::Controller::Controller() {
		open();
		signal(SIGHUP,handle_reload);
	}

	Config::Controller::~Controller() {
		signal(SIGHUP,SIG_DFL);
		close();
	}

	void Config::Controller::open() {

		std::lock_guard<std::recursive_mutex> lock(guard);

		if(hFile) {
			return;
		}

		econf_err err = ECONF_ERROR;

#ifdef ECONF_HAVE_READ_CONFIG_WITH_CALLBACK
		err = econf_readConfigWithCallback(
			&hFile,
			NULL,
			"/usr/lib",
			program_invocation_short_name,
			"conf",
			"=", "#",
			checkFile,
			NULL
		);

		debug("err=",err," allow_user_config=",allow_user_config);
		if(err == ECONF_NOFILE && allow_user_config) {

			debug("Trying user configuration directory");

			const char *homedir = getenv("HOME");
			if(homedir) {
				if(hFile) {
					econf_freeFile(hFile);
					hFile = nullptr;
				}
				debug("Trying user's home dir");
				err = econf_readConfigWithCallback(
					&hFile,
					NULL,
					String{(const char *) homedir,"/.local/etc"}.c_str(),
					program_invocation_short_name,
					"conf",
					"=", "#",
					checkFile,
					NULL
				);
			}
		}
#else
		err = econf_readDirs(
			&hFile,								// key_file
			"/usr/etc",							// usr_conf_dir
			"/etc",								// etc_conf_dir
			program_invocation_short_name,		// Application name
			".conf",							// config_suffis
			"=",								// delim
			"#"									// comment
		);
#endif
		if(err != ECONF_SUCCESS) {
			if(hFile) {
				econf_freeFile(hFile);
				hFile = nullptr;
			}
			Logger::String{"Cant load configuration (",econf_errString(err),"), using defaults"}.warning();
		}

	}

	void Config::Controller::close() {
		std::lock_guard<std::recursive_mutex> lock(guard);
		if(hFile) {
			econf_freeFile(hFile);
			hFile = nullptr;
		}
	}

	void Config::Controller::reload() {
		close();
		open();
	}

	bool Config::Controller::hasGroup(const char *group) {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return false;
		}

		bool rc = false;
		size_t length = 0;
		char **groups;

		econf_err err = econf_getGroups(hFile, &length, &groups);

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		for(size_t ix = 0; ix < length; ix++) {
			if(!strcasecmp(groups[ix],group)) {
				rc = true;
				break;
			}
		}
		econf_freeArray(groups);

		return rc;

	}

	bool Config::Controller::hasKey(const char *group, const char *key) {

		if(!hasGroup(group)) {
			return false;
		}

		std::lock_guard<std::recursive_mutex> lock(guard);

		bool rc = false;

		size_t length = 0;
		char **keys;

		// Check for group
		econf_err err = econf_getKeys(hFile, group, &length, &keys);

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		for(size_t ix = 0; ix < length; ix++) {
			if(!strcasecmp(keys[ix],key)) {
				rc = true;
				break;
			}
		}
		econf_freeArray(keys);

		return rc;
	}

	int32_t Config::Controller::get(const char *group, const char *name, const int32_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		int32_t result;
		econf_err err = econf_getIntValueDef(
							hFile,
							group,
							name,
							&result,
							def
						);

		if(err == ECONF_NOKEY)
			return def;

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		return result;

	}

	int64_t Config::Controller::get(const char *group, const char *name, const int64_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		int64_t result;

		econf_err err = econf_getInt64ValueDef(
							hFile,
							group,
							name,
							&result,
							def
						);

		if(err == ECONF_NOKEY)
			return def;

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		return result;
	}

	uint32_t Config::Controller::get(const char *group, const char *name, const uint32_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		uint32_t result;

		econf_err err = econf_getUIntValueDef(
							(econf_file *) hFile,
							group,
							name,
							&result,
							def
						);

		if(err == ECONF_NOKEY)
			return def;

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		return result;
	}

	uint64_t Config::Controller::get(const char *group, const char *name, const uint64_t def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		uint64_t result;

		econf_err err = econf_getUInt64ValueDef(
							(econf_file *) hFile,
							group,
							name,
							&result,
							def
						);

		if(err == ECONF_NOKEY)
			return def;

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		return result;

	}

	float Config::Controller::get(const char *group, const char *name, const float def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		float result;

		econf_err err = econf_getFloatValueDef(
							(econf_file *) hFile,
							group,
							name,
							&result,
							def
						);

		if(err == ECONF_NOKEY)
			return def;

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		return result;

	}

	double Config::Controller::get(const char *group, const char *name, const double def) const {

		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		double result;

		econf_err err = econf_getDoubleValueDef(
							(econf_file *) hFile,
							group,
							name,
							&result,
							def
						);

		if(err == ECONF_NOKEY)
			return def;

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		return result;

	}

	bool Config::Controller::get(const char *group, const char *name, const bool def) const {

		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		bool result;

		econf_err err = econf_getBoolValueDef(
							(econf_file *) hFile,
							group,
							name,
							&result,
							def
						);

		if(err == ECONF_NOKEY)
			return def;

		if(err != ECONF_SUCCESS)
			throw std::runtime_error(econf_errString(err));

		return result;

	}

	Udjat::String Config::Controller::get_string(const char *group, const char *name, const char *def) const {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return def;
		}

		char *result = NULL;

		econf_err err = econf_getStringValueDef(
							(econf_file *) hFile,
							group,
							name,
							&result,
							(char *) def
						);


		if(err != ECONF_SUCCESS) {

			if(result) {
				free(result);
			}

			if(err == ECONF_NOKEY) {
				return def;
			}

			throw std::runtime_error(econf_errString(err));

		}

		if(result) {
			String rc{result};
			free(result);
			return rc;
		}

		Logger::String{"Empty value for '", group, ".", name, "'"}.info();
		return def;

	}

	bool Config::Controller::for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {
		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {
			return false;
		}

		// Get keys.
		size_t length = 0;
		char **keys;
		econf_err err = econf_getKeys((econf_file *) hFile, group, &length, &keys);

		if(err != ECONF_SUCCESS) {
			if(err == ECONF_NOKEY) {
				Logger::String("No '",group,"' section on configuration file").write(Logger::Debug,"econf");
				return false;
			}
			throw std::runtime_error(econf_errString(err));

		}

		bool found = false;
		for(size_t ix = 0;ix < length && !found; ix++) {

			char *value = nullptr;
			err = econf_getStringValueDef(
						(econf_file *) hFile,
						group,
						keys[ix],
						&value,
						(char *) ""		// It should be const here but, it isnt in libeconf.
					);

			if(err == ECONF_SUCCESS) {
				try {
					found = call(keys[ix],value);
					debug("group='",group,"' key='",keys[ix],"' value='",value,"' found=",found);
				} catch(const std::exception &e) {
					Logger::String{"Error '", e.what(), "' navigating from configuration"}.error();
					found = true; // Stop iterating.
				} catch(...) {
					Logger::String{"Unexpected error navigating from configuration"}.error();
					found = true; // Stop iterating.
				}
			}

			if(value) {
				free(value);
			}

		}
		econf_freeArray(keys);

		debug("for_each group='",group,"' found=",found);
		return found;

	}

 }

 #endif // HAVE_ECONF


