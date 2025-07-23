/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Brief Implements common config methods.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/configuration.h>
 #include <udjat/tools/configuration.h>
 #include <cstdarg>
 #include <string>
 #include <cstring>

 using namespace std;

 namespace Udjat {

	namespace Config {

		bool Controller::allow_user_config = false;

#ifdef _WIN32
		Controller & Controller::getInstance() {
			static Config::Controller instance;
			return instance;
		}

		bool Controller::allow_user_homedir(bool allow);
			HKEY hNew = allow ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
			if(hNew == hParent) {
				return false;
			}
			hParent = hNew;
			Logger::String{"Configuration from user homedir is ",string(allow ? "enabled" : "disabled")}.trace();
			return true;
		}

#else

		Controller & Controller::getInstance() {
			std::lock_guard<std::recursive_mutex> lock(guard);
			static Config::Controller instance;
			return instance;
		}

		bool Controller::allow_user_homedir(bool allow) {
			if(allow == allow_user_config) {
				return false;
			}
			allow_user_config = allow;
			Logger::String{"Configuration from user homedir is ",string(allow ? "enabled" : "disabled")}.trace();
			return true;
		}
#endif

		int Value<string>::select(const char *value, va_list args) const noexcept {

			if(empty()) {
				return -(errno = ENODATA);
			}

			int index = 0;
			while(value) {

				if(!strcasecmp(c_str(),value)) {
					va_end(args);
					return index;
				}

				index++;
				value = va_arg(args, const char *);
			}
			return -(errno = ENOENT);

		}

		int Value<string>::select(const char *value, ...) const noexcept {

			va_list args;
			va_start(args, value);
			int rc = select(value,args);
			va_end(args);

			return rc;

		}

		UDJAT_API void allow_user_homedir(bool allow) noexcept {
			Controller::allow_user_homedir(allow);
		}

		UDJAT_API bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {
			return Controller::getInstance().for_each(group,call);
		}

		UDJAT_API bool hasGroup(const std::string &group) {
			return Controller::getInstance().hasGroup(group.c_str());
		}

		UDJAT_API bool hasKey(const char *group, const char *key) {
			return Controller::getInstance().hasKey(group,key);
		}

		UDJAT_API int32_t get(const std::string &group, const std::string &name, const int32_t def) {
			return Controller::getInstance().get(group.c_str(),name.c_str(),def);
		}

		UDJAT_API int64_t get(const std::string &group, const std::string &name, const int64_t def) {
			return Controller::getInstance().get(group.c_str(),name.c_str(),def);
		}

		UDJAT_API uint32_t get(const std::string &group, const std::string &name, const uint32_t def) {
			return Controller::getInstance().get(group.c_str(),name.c_str(),def);
		}

		UDJAT_API uint64_t get(const std::string &group, const std::string &name, const uint64_t def) {
			return Controller::getInstance().get(group.c_str(),name.c_str(),def);
		}

		UDJAT_API float get(const std::string &group, const std::string &name, const float def) {
			return Controller::getInstance().get(group.c_str(),name.c_str(),def);
		}

		UDJAT_API double get(const std::string &group, const std::string &name, const double def) {
			return Controller::getInstance().get(group.c_str(),name.c_str(),def);
		}

		UDJAT_API Udjat::String get(const std::string &group, const std::string &name, const std::string &def) {
			return get(group,name,def.c_str());
		}

		UDJAT_API bool get(const std::string &group, const std::string &name, const bool def) {
			return Controller::getInstance().get(group.c_str(),name.c_str(),def);
		}

		UDJAT_API Udjat::String get(const std::string &group, const std::string &name, const char *def) {
			String str = Controller::getInstance().get_string(group.c_str(),name.c_str(),def);
			return str.expand(true,false);
		}

	}

 }
