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

 #ifdef LOG_DOMAIN
	#undef LOG_DOMAIN
 #endif
 #define LOG_DOMAIN "config"
 #include <udjat/tools/logger.h>
 
 #include <udjat/tools/string.h>
 #include <signal.h>
 #include <iostream>
 #include <cstring>
 #include <limits.h>
 #include <unistd.h>
 #include <vector>

 // https://github.com/openSUSE/libeconf
 #if defined(HAVE_ECONF)
 extern "C" {
	#include <libeconf.h>
 }
 #elif defined(HAVE_INIPARSER)
	#include <iniparser.h>
 #endif // HAVE_ECONF

 using namespace std;

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

		int32_t get(const char *group, const char *name, const int32_t def) const {

			if(!hFile) {
				return def;
			}

			int32_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getIntValueDef(
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

		int64_t get(const char *group, const char *name, const int64_t def) const {

			if(!hFile) {
				return def;
			}

			int64_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

			econf_err err = econf_getInt64ValueDef(
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

		uint32_t get(const char *group, const char *name, const uint32_t def) const {

			if(!hFile) {
				return def;
			}

			uint32_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

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

		uint64_t get(const char *group, const char *name, const uint64_t def) const {

			if(!hFile) {
				return def;
			}

			uint64_t result;

			std::lock_guard<std::recursive_mutex> lock(guard);

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

		float get(const char *group, const char *name, const float def) const {

			if(!hFile) {
				return def;
			}

			float result;

			std::lock_guard<std::recursive_mutex> lock(guard);

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

		double get(const char *group, const char *name, const double def) const {

			if(!hFile) {
				return def;
			}

			double result;

			std::lock_guard<std::recursive_mutex> lock(guard);

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

		bool get(const char *group, const char *name, const bool def) const {

			if(!hFile) {
				return def;
			}

			bool result;

			std::lock_guard<std::recursive_mutex> lock(guard);

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

		Udjat::String get_string(const char *group, const char *name, const char *def) const {

			if(!hFile) {
				return def;
			}

			char *result = NULL;

			std::lock_guard<std::recursive_mutex> lock(guard);

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
				string rc{result};
				free(result);
				return rc;
			}

			cout << "config\tEmpty value for '" << group << "." << name << "'" << endl;
			return def;

		}

		Udjat::String get_string(const char *group, const char *name, const std::string &def) const {
			return get_string(group,name,def.c_str());
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


#elif defined(HAVE_INIPARSER)

	//
	// Use iniparser as back-end
	//
	class Controller {
		private:
			dictionary  *ini;

			Controller() : ini{iniparser_load(String{"/etc/",program_invocation_short_name,".conf"}.c_str())} {
				if(!ini) {
					Logger::String{"Cant load /etc/",program_invocation_short_name,".conf, using default configuration"}.warning("iniparser");
				} else if(Logger::enabled(Logger::Trace)) {
					Logger::String{"Got configuration from /etc/",program_invocation_short_name,".conf'"}.trace("iniparser");
				}
			}

			Udjat::String key(const char *group, const char *key) const {
				return Udjat::String{group,":",key};
			}

		public:
			~Controller() {
				if(ini) {
					iniparser_freedict(ini);
					ini = nullptr;
				}
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
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return false;
				}
				return (iniparser_getsecnkeys(ini,(char *)group) != 0);
			}
	
			bool hasKey(const char *, const char *) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				return true;
			}

			Udjat::String get_string(const char *group, const char *name, const char *def) const {
				std::lock_guard<std::recursive_mutex> lock(guard);

				debug("group='",group,"' name='",name,"'");
				if(!ini) {
					return Udjat::String{def};
				}
				String str{iniparser_getstring(ini,key(group,name).c_str(),(char *) def)};

				debug(group,":",name,"='",str.c_str(),"'");
				return str;
			}
			
			Udjat::String get_string(const char *group, const char *name, const std::string &def) const {
				return get_string(group,name,def.c_str());
			}
	
			int32_t get(const char *group, const char *name, const int32_t def) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return def;
				}
				return iniparser_getint(ini,key(group,name).c_str(),def);
			}
		
			int64_t get(const char *group, const char *name, const int64_t def) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return def;
				}
				return iniparser_getint(ini,key(group,name).c_str(),def);
			}
		
			uint32_t get(const char *group, const char *name, const uint32_t def) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return def;
				}
				return iniparser_getint(ini,key(group,name).c_str(),def);
			}
		
			uint64_t get(const char *group, const char *name, const uint64_t def) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return def;
				}
				return (unsigned int) iniparser_getint(ini,key(group,name).c_str(),def);
			}
		
			bool get(const char *group, const char *name, const bool def) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return def;
				}
				return iniparser_getboolean(ini,key(group,name).c_str(),def);
			}
			
			float get(const char *group, const char *name, const float def) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return def;
				}
				return iniparser_getdouble(ini,key(group,name).c_str(),def);
			}
		
			double get(const char *group, const char *name, const double def) {
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(!ini) {
					return def;
				}
				return iniparser_getdouble(ini,key(group,name).c_str(),def);
			}
		
			bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {

				throw system_error(ENOTSUP,system_category(),"Cant enumerate iniparser keys on legacy mode");

				/*
				std::lock_guard<std::recursive_mutex> lock(guard);
				if(ini) {
				  size_t items = iniparser_getsecnkeys(ini,(char *) group);
				  if(items) {
				    char **k = iniparser_getseckeys(ini, (char *) group);
				    if(k) {
				      for(size_t ix = 0; ix < items; ix++) {
					const char *ptr = strchr(k[ix],':');
					  if(ptr) {
					  ptr++;
					  if(call(ptr,Config::get(group,ptr,"").c_str())) {
					    return true;
					  }
					}
				      }
				    }
				  }
				}
*/

				return false;
			}
	
		};
	
#else

	//
	// No Back-end, Use a dumb controller.
	//

	class Controller {
	private:
		Controller() {
			Logger::String{"No config file backend, using internal defaults"}.warning();
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

		Udjat::String get_string(const char *group, const char *name, const char *def) const {
			return Udjat::String{def};
		}

		inline Udjat::String get_string(const char *group, const char *name, const std::string &def) const {
			return Udjat::String{def.c_str()};
		}

		template<typename T>
		T get(const char *group, const char *name, const T def) const {
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
		return Controller::getInstance().get(group.c_str(),name.c_str(),def);
	}

	int64_t Config::get(const std::string &group, const std::string &name, const int64_t def) {
		return Controller::getInstance().get(group.c_str(),name.c_str(),def);
	}

	uint32_t Config::get(const std::string &group, const std::string &name, const uint32_t def) {
		return Controller::getInstance().get(group.c_str(),name.c_str(),def);
	}

	uint64_t Config::get(const std::string &group, const std::string &name, const uint64_t def) {
		return Controller::getInstance().get(group.c_str(),name.c_str(),def);
	}

	float Config::get(const std::string &group, const std::string &name, const float def) {
		return Controller::getInstance().get(group.c_str(),name.c_str(),def);
	}

	double Config::get(const std::string &group, const std::string &name, const double def) {
		return Controller::getInstance().get(group.c_str(),name.c_str(),def);
	}

	Udjat::String Config::get(const std::string &group, const std::string &name, const char *def) {
		String str = Controller::getInstance().get_string(group.c_str(),name.c_str(),def);
		return str.expand(true,false);
	}

	Udjat::String Config::get(const std::string &group, const std::string &name, const std::string &def) {
		String str = Controller::getInstance().get_string(group.c_str(),name.c_str(),def);
		return str.expand(true,false);
	}

	bool Config::get(const std::string &group, const std::string &name, const bool def) {
		return Controller::getInstance().get(group.c_str(),name.c_str(),def);
	}

 }



