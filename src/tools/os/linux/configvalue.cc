/**
 *
 * Copyright (C) <2017> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @file
 *
 * @brief Implements linux configuration file.
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 */

 #include <config.h>
 #include <udjat/tools/configuration.h>
 #include <signal.h>
 #include <iostream>
 #include <cstring>

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

#ifdef HAVE_ECONF
	class Controller {
	private:
		void *hFile = nullptr;	///< @brief Configuration file handle.

		static void handle_reload(int sig) noexcept {

			clog << "config\tReloading configuration by signal '" << strsignal(sig) << "'" << endl;

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

		void open() {

			std::lock_guard<std::recursive_mutex> lock(guard);
			string confname{"udjat"};

			if(!hFile) {

				econf_err err = econf_readDirs(
					(econf_file **) &hFile,		// key_file
					"/usr/etc",					// usr_conf_dir
					"/etc",						// etc_conf_dir
					confname.c_str(),			// project_name
					".conf",					// config_suffis
					"=",						// delim
					"#"							// comment
				);

				if(err != ECONF_SUCCESS) {
					hFile = nullptr;
					throw std::runtime_error(econf_errString(err));
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

		bool hasGroup(const std::string &group) {
			std::lock_guard<std::recursive_mutex> lock(guard);
			bool rc = false;

			size_t length = 0;
			char **groups;
			econf_err err = econf_getGroups((econf_file *) hFile, &length, &groups);
			if(err != ECONF_SUCCESS)
				throw std::runtime_error(econf_errString(err));
			for(size_t ix = 0; ix < length; ix++) {
				if(!strcasecmp(groups[ix],group.c_str())) {
					rc = true;
					break;
				}
			}
			econf_freeArray(groups);
			return rc;
		}

		int32_t get(const std::string &group, const std::string &name, const int32_t def) const {

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
	//				cout << "config\tNo configuration for '" << group << "." << name << "'" << endl;
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


	};
#endif // HAVE_ECONF

	bool Config::hasGroup(const std::string &group) {
		return Controller::getInstance().hasGroup(group);
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



