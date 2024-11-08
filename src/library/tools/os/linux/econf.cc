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

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif // !_GNU_SOURCE

 #include <config.h>
 #include <errno.h>
 #include <private/configuration.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <signal.h>
 #include <iostream>
 #include <cstring>
 #include <limits.h>
 #include <unistd.h>

 // https://github.com/openSUSE/libeconf
 #ifdef HAVE_ECONF
 extern "C" {
	#include <libeconf.h>
 }
 #endif // HAVE_ECONF

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	static recursive_mutex guard;
	static bool allow_user_config = false;

	void Config::allow_user_homedir(bool allow) noexcept {
		allow_user_config = allow;
	}

#if defined(HAVE_ECONF)

	//
	// Use libeconf as back-end
	//
	class Controller {
	private:
		void *hFile = nullptr;		///< @brief Configuration file handle.

		static void handle_reload(int sig) noexcept {

			Logger::String("Reloading configuration by signal '",(const char *) strsignal(sig),"'").write(Logger::Trace,"econf");

			try {

				getInstance().reload();

			} catch(const std::exception &e) {

				cerr << "config\tError '" << e.what() << "' reloading configuration" << endl;

			} catch(...) {

				cerr << "config\tUnexpected error reloading configuration" << endl;

			}

		}

		Controller() : hFile(nullptr) {
			open();
			signal(SIGHUP,handle_reload);
		}


	public:
		~Controller() {
			signal(SIGHUP,SIG_DFL);
			close();
		}

		static Controller & getInstance() {
			std::lock_guard<std::recursive_mutex> lock(guard);
			static Controller instance;
			return instance;
		}

		void reload() {
			std::lock_guard<std::recursive_mutex> lock(guard);
			close();
			open();
		}

#ifdef ECONF_HAVE_READ_CONFIG_WITH_CALLBACK
		static bool checkFile(const char *filename, const void *) {
			Logger::String{"Loading '",filename,"'"}.trace("econf");
			return true;
		}
#endif

		void open() {

			std::lock_guard<std::recursive_mutex> lock(guard);

			if(!hFile) {

				econf_err err = ECONF_ERROR;

				debug("--------------------------------------------------");

#ifdef ECONF_HAVE_READ_CONFIG_WITH_CALLBACK
				err = econf_readConfigWithCallback(
					(econf_file **) &hFile,
					NULL,
					"/usr/lib",
					program_invocation_short_name,
					"conf",
					"=", "#",
					checkFile,
					NULL
				);

				debug("err=",err);
				if(err == ECONF_NOFILE && allow_user_config) {
					const char *homedir = getenv("HOME");
					if(homedir) {
						if(hFile) {
							econf_freeFile((econf_file *) hFile);
							hFile = nullptr;
						}
						debug("Trying user's home dir");
						err = econf_readConfigWithCallback(
							(econf_file **) &hFile,
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
					(econf_file **) &hFile,				// key_file
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
						econf_freeFile((econf_file *) hFile);
						hFile = nullptr;
					}
					Logger::String{"Cant load configuration (",econf_errString(err),"), using defaults"}.warning("econf");
				}
			}

		}

		void close() {
			std::lock_guard<std::recursive_mutex> lock(guard);
			if(hFile) {
				econf_freeFile((econf_file *) hFile);
				hFile = nullptr;
			}
		}

		bool hasGroup(const char *group) {

			std::lock_guard<std::recursive_mutex> lock(guard);
			bool rc = false;

			if(hFile) {
				size_t length = 0;
				char **groups;
				econf_err err = econf_getGroups((econf_file *) hFile, &length, &groups);
				if(err != ECONF_SUCCESS)
					throw std::runtime_error(econf_errString(err));
				for(size_t ix = 0; ix < length; ix++) {
					if(!strcasecmp(groups[ix],group)) {
						rc = true;
						break;
					}
				}
				econf_freeArray(groups);
			}

			return rc;
		}

		bool hasKey(const char *group, const char *key) {

			std::lock_guard<std::recursive_mutex> lock(guard);

			if(!hasGroup(group)) {
				return false;
			}

			bool rc = false;

			size_t length = 0;
			char **keys;

			// Check for group
			econf_err err = econf_getKeys((econf_file *) hFile, group, &length, &keys);

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

		int32_t get(const std::string &group, const std::string &name, const int32_t def) const {

			if(!hFile) {
				return def;
			}

			int32_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getIntValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
								&result,
								def
							);

			if(err == ECONF_NOKEY)
				return def;

			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));

			return result;

		}

		int64_t get(const std::string &group, const std::string &name, const int64_t def) const {

			if(!hFile) {
				return def;
			}

			int64_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getInt64ValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
								&result,
								def
							);

			if(err == ECONF_NOKEY)
				return def;

			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));

			return result;

		}

		uint32_t get(const std::string &group, const std::string &name, const uint32_t def) const {

			if(!hFile) {
				return def;
			}

			uint32_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getUIntValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
								&result,
								def
							);

			if(err == ECONF_NOKEY)
				return def;

			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));

			return result;
		}

		uint64_t get(const std::string &group, const std::string &name, const uint64_t def) const {

			if(!hFile) {
				return def;
			}

			uint64_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getUInt64ValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
								&result,
								def
							);

			if(err == ECONF_NOKEY)
				return def;

			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));

			return result;
		}

		float get(const std::string &group, const std::string &name, const float def) const {

			if(!hFile) {
				return def;
			}

			float result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getFloatValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
								&result,
								def
							);

			if(err == ECONF_NOKEY)
				return def;

			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));

			return result;
		}

		double get(const std::string &group, const std::string &name, const double def) const {

			if(!hFile) {
				return def;
			}

			double result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getDoubleValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
								&result,
								def
							);

			if(err == ECONF_NOKEY)
				return def;

			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));

			return result;

		}

		bool get(const std::string &group, const std::string &name, const bool def) const {

			if(!hFile) {
				return def;
			}

			bool result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getBoolValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
								&result,
								def
							);

			if(err == ECONF_NOKEY)
				return def;

			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));

			return result;

		}

		std::string get(const std::string &group, const std::string &name, const char *def) const {

			if(!hFile) {
				return def;
			}

			char *result = NULL;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getStringValueDef(
								(econf_file *) hFile,
								group.c_str(),
								name.c_str(),
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
				string rc{result};
				free(result);
				return rc;
			}

			cout << "config\tEmpty value for '" << group << "." << name << "'" << endl;
			return def;

		}

		std::string get(const std::string &group, const std::string &name, const std::string &def) const {
			return get(group,name,def.c_str());
		}

		bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {

			if(!hFile) {
				return false;
			}

			std::lock_guard<std::recursive_mutex> lock(guard);

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

			bool next = true;
			for(size_t ix = 0;ix < length && next; ix++) {

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

						next = call(keys[ix],value);

					} catch(const std::exception &e) {
						cerr << "config\tError '" << e.what() << "' navigating from configuration" << endl;
						next = false;
					} catch(...) {
						cerr << "config\tUnexpected error navigating from configuration" << endl;
						next = false;
					}

				}

				if(value) {
					free(value);
				}

			}
			econf_freeArray(keys);

			return next;

		}

	};


#else

	//
	// No Back-end, Use a dumb controller.
	//

	class Controller {
	private:
		Controller() {
			cerr << "config\tNo config file backend, using internal defaults" << endl;
		}


	public:
		~Controller() {
		}

		static Controller & getInstance() {
			std::lock_guard<std::recursive_mutex> lock(guard);
			static Controller instance;
			return instance;
		}

		void reload() {
		}

		void open() {
		}

		void close() {
		}

		bool hasGroup(const char *group) {
			return false;
		}

		bool hasKey(const char *group, const char *key) {
			return false;
		}

		std::string get(const std::string &group, const std::string &name, const char *def) const {
			return string(def);
		}

		std::string get(const std::string &group, const std::string &name, const std::string &def) const {
			return string(def);
		}

		template<typename T>
		T get(const std::string &group, const std::string &name, const T def) const {
			return def;
		}

		bool for_each(const char *group,const std::function<void(const char *key, const char *value)> &call) {
			return false;
		}

	};
#endif // HAVE_ECONF

	bool Config::for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {
		return Controller::getInstance().for_each(group,call);
	}

	bool Config::hasGroup(const std::string &group) {
		return Controller::getInstance().hasGroup(group.c_str());
	}

	bool Config::hasKey(const char *group, const char *key) {
		return Controller::getInstance().hasKey(group,key);
	}

	int32_t Config::get(const std::string &group, const std::string &name, const int32_t def) {
		return Controller::getInstance().get(group,name,def);
	}

	int64_t Config::get(const std::string &group, const std::string &name, const int64_t def) {
		return Controller::getInstance().get(group,name,def);
	}

	uint32_t Config::get(const std::string &group, const std::string &name, const uint32_t def) {
		return Controller::getInstance().get(group,name,def);
	}

	uint64_t Config::get(const std::string &group, const std::string &name, const uint64_t def) {
		return Controller::getInstance().get(group,name,def);
	}

	float Config::get(const std::string &group, const std::string &name, const float def) {
		return Controller::getInstance().get(group,name,def);
	}

	double Config::get(const std::string &group, const std::string &name, const double def) {
		return Controller::getInstance().get(group,name,def);
	}

	std::string Config::get(const std::string &group, const std::string &name, const char *def) {
		return Controller::getInstance().get(group,name,def);
	}

	std::string Config::get(const std::string &group, const std::string &name, const std::string &def) {
		return Controller::getInstance().get(group,name,def);
	}

	bool Config::get(const std::string &group, const std::string &name, const bool def) {
		return Controller::getInstance().get(group,name,def);
	}

 }



