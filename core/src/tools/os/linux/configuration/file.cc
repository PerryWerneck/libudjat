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
 extern "C" {
	#include <libeconf.h>
 }

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	recursive_mutex Config::File::guard;

	Config::File & Config::File::getInstance()  {
		static Config::File file;
		return file;
	}

	Config::File::File() : hFile(nullptr) {

		open();
		signal(SIGHUP,handle_reload);

	}

	Config::File::~File() {

		signal(SIGHUP,SIG_DFL);
		close();

	}

	void Config::File::reload() {

		// Fecha e abre arquivos de configuração.
		std::lock_guard<std::recursive_mutex> lock(guard);

		close();
		open();

	}

	void Config::File::handle_reload(int sig) noexcept {
		clog << "config\tReloading configuration by signal '" << strsignal(sig) << "'" << endl;

		try {

			getInstance().reload();

		} catch(const std::exception &e) {

			cerr << "config\tError '" << e.what() << "' reloading configuration" << endl;

		} catch(...) {

			cerr << "config\tUnexpected error reloading configuration" << endl;

		}

	}

	void Config::File::open() {

		std::lock_guard<std::recursive_mutex> lock(guard);

		if(!hFile) {

			econf_err err = econf_readDirs(
				(econf_file **) &hFile,
				"/usr/etc",
				"/etc",
#ifdef PRODUCT_NAME
				STRINGIZE_VALUE_OF(PRODUCT_NAME),
#else
				PACKAGE_NAME,
#endif // PRODUCT_NAME
				".conf",
				"=",
				"#"
			);

			if(err != ECONF_SUCCESS) {
				hFile = nullptr;
				throw std::runtime_error(econf_errString(err));
			}

		}

	}

	void Config::File::close() {

		std::lock_guard<std::recursive_mutex> lock(guard);

		if(hFile) {

			econf_freeFile((econf_file *) hFile);
			hFile = nullptr;

		}

	}

	int32_t Config::File::get(const std::string &group, const std::string &name, const int32_t def) const {

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

	int64_t Config::File::get(const std::string &group, const std::string &name, const int64_t def) const {

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

	uint32_t Config::File::get(const std::string &group, const std::string &name, const uint32_t def) const {

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

	uint64_t Config::File::get(const std::string &group, const std::string &name, const uint64_t def) const {

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

	float Config::File::get(const std::string &group, const std::string &name, const float def) const {

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

	double Config::File::get(const std::string &group, const std::string &name, const double def) const {

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

	bool Config::File::get(const std::string &group, const std::string &name, const bool def) const {

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

	std::string Config::File::get(const std::string &group, const std::string &name, const char *def) const {

		char *result = NULL;

		std::lock_guard<std::recursive_mutex> lock(guard);

		econf_err err = econf_getStringValueDef(
							(econf_file *) hFile,
							group.c_str(),
							name.c_str(),
							&result,
							(char *) def
						);


		if(err != ECONF_SUCCESS && err != ECONF_NOKEY) {
			if(result) {
				free(result);
			}
			throw std::runtime_error(econf_errString(err));
		}

		std::string rc;

		if(result) {
			rc.assign(result);
			free(result);
		}

		return rc;

	}

	std::string Config::File::get(const std::string &group, const std::string &name, const std::string &def) const {
		return get(group,name,def.c_str());
	}


 }



