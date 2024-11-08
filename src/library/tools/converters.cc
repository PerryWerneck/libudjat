/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implement converters.
  */

 #include <config.h>
 #include <udjat/tools/converters.h>
 #include <udjat/defs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

 	template <>
	UDJAT_API bool from_string<bool>(const char *str) {

		if(!(str && *str)) {
			return false;
		}

		static const char * yes[] = {
			N_("yes"),
			N_("true"),
			N_("on"),
			"1"
		};

		static const char * no[] = {
			N_("no"),
			N_("false"),
			N_("off"),
			"0"
		};

		for(const char *ptr : yes) {

			if(strcasecmp(str,ptr) == 0) {
				return true;
			}
#ifdef GETTEXT_PACKAGE
			if(strcasecmp(str,dgettext(GETTEXT_PACKAGE,ptr)) == 0) {
				return true;
			}
#endif // GETTEXT_PACKAGE
		}

		for(const char *ptr : no) {
			if(strcasecmp(str,ptr) == 0) {
				return false;
			}
#ifdef GETTEXT_PACKAGE
			if(strcasecmp(str,dgettext(GETTEXT_PACKAGE,ptr)) == 0) {
				return false;
			}
#endif // GETTEXT_PACKAGE

		}

		Logger::String{"Unexpected boolean '",str,"' assuming 'false'"}.warning("string");
		return false;

	}

 }

