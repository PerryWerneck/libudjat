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

 #pragma once
 #include <udjat/defs.h>
#include <string>

 namespace Udjat {

	/// @brief Module information data.
	struct UDJAT_API ModuleInfo {

		// Module build - SHOULD BE THE FIRST VALUE.
#ifdef BUILD_DATE
		const int build = BUILD_DATE;
#else
		const int build = 0;
#endif // BUILD_DATE

		/// @brief The module name.
#ifdef PACKAGE_NAME
		const char *name = PACKAGE_NAME;
#else
		const char *name = "module";
#endif // PACKAGE_NAME

		/// @brief The module description.
#ifdef PACKAGE_DESCRIPTION
		const char *description = PACKAGE_DESCRIPTION;
#else
		const char *description = "";
#endif // PACKAGE_DESCRIPTION

		/// @brief The module version.
#ifdef PACKAGE_VERSION
		const char *version = PACKAGE_VERSION;
#else
		const char *version = "";
#endif // PACKAGE_VERSION

		/// @brief The bugreport address.
#ifdef PACKAGE_BUGREPORT
		const char *bugreport = PACKAGE_BUGREPORT;
#else
		const char *bugreport = "";
#endif // PACKAGE_BUGREPORT

		/// @brief The package URL.
#ifdef PACKAGE_URL
		const char *url = PACKAGE_URL;
#else
		const char *url = "";
#endif // PACKAGE_URL

#ifdef GETTEXT_PACKAGE
		const char * gettext_package = GETTEXT_PACKAGE;
#else
		const char * gettext_package = nullptr;
#endif // GETTEXT_PACKAGE

// https://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations
#ifdef __cpp_constexpr

#ifdef PACKAGE_DESCRIPTION
		constexpr ModuleInfo() {
		}
#endif // PACKAGE_DESCRIPTION

		constexpr ModuleInfo(const char *d) :
			description(d) { }

		constexpr ModuleInfo(const char *n, const char *d) :
			name(n), description(d) { }

		constexpr ModuleInfo(const char *n, const char *d, const char *v, const char *u, const char *b= "") :
			name(n), description(d), version(v), bugreport(b), url(u) { }
#else
		ModuleInfo(const char *d) : description(d) {
		}

		ModuleInfo(const char *n, const char *d, const char *v = "", const char *u="", const char *b= "") :
			name(n), description(d), version(v), bugreport(b), url(u) { }
#endif

		Udjat::Value & get(Udjat::Value &value) const [[deprecated]];

		/// @brief Get property value.
		/// @param key The property name.
		/// @param value String to update with the property value.
		/// @return true if the property is valid.
		bool getProperty(const char *key, std::string &value) const;

		Value & getProperties(Value &properties) const;

	};

 }

