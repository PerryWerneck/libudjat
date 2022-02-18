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

/**
 * @file
 *
 * @brief Implements windows configuration file abstraction.
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/quark.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/registry.h>
 #include <udjat/tools/logger.h>
 #include <signal.h>
 #include <iostream>
 #include <cstring>

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	UDJAT_API bool Config::for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {
		return Win32::Registry().for_each(group,call);
	}

	UDJAT_API bool Config::hasGroup(const std::string &group) {
		return Win32::Registry().hasKey(group.c_str());
	}

	UDJAT_API bool Config::hasKey(const char *group, const char *key) {
		return Win32::Registry(group).hasValue(key);
	}

	UDJAT_API int32_t Config::get(const std::string &group, const std::string &name, const int32_t def) {
		return (int32_t) Win32::Registry(group.c_str()).get(name.c_str(),(DWORD) def);
	}

	UDJAT_API uint32_t Config::get(const std::string &group, const std::string &name, const uint32_t def) {
		return (uint32_t) Win32::Registry(group.c_str()).get(name.c_str(),(DWORD) def);
	}

	UDJAT_API std::string Config::get(const std::string &group, const std::string &name, const char *def) {
		return Win32::Registry(group.c_str()).get(name.c_str(),def);
	}

	UDJAT_API std::string Config::get(const std::string &group, const std::string &name, const std::string &def) {
		return get(group,name,def.c_str());
	}

	UDJAT_API bool Config::get(const std::string &group, const std::string &name, const bool def) {
		return (Win32::Registry(group.c_str()).get(name.c_str(),(DWORD) def) != 0);
	}

	UDJAT_API int64_t Config::get(const std::string &group, const std::string &name, const int64_t def) {
		// FIXME: Implement.
		return def;
	}

	UDJAT_API uint64_t Config::get(const std::string &group, const std::string &name, const uint64_t def) {
		// FIXME: Implement.
		return def;
	}

	UDJAT_API float Config::get(const std::string &group, const std::string &name, const float def) {
		// FIXME: Implement.
		return def;
	}

	UDJAT_API double Config::get(const std::string &group, const std::string &name, const double def) {
		// FIXME: Implement.
		return def;
	}


 }



