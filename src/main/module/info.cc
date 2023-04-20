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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/intl.h>
 #include <udjat/moduleinfo.h>
 #include <iostream>
 #include <cstdarg>

 using namespace std;

 namespace Udjat {

	Value & ModuleInfo::get(Value &value) const {
		return getProperties(value);
	}

	Value & ModuleInfo::getProperties(Value &properties) const {

		struct {
			const char *name;
			const char *value;
		} info[] = {
			{ "module",			name		},	// Use 'modulename' to keep the child object name.
			{ "description",	description	},
			{ "version",		version		},
			{ "bugreport",		bugreport	},
			{ "url",			url			},
		};

		for(auto &item : info) {
			if(gettext_package && *gettext_package) {
				properties[item.name] = (const char *) dgettext(gettext_package,item.value);
			} else {
				properties[item.name] = item.value;
			}
		}

		properties["build"] = build;
		if(gettext_package) {
			properties["locale"] = gettext_package;
		}

		return properties;
	}

	bool ModuleInfo::getProperty(const char *key, std::string &value) const {

		struct {
			const char *name;
			const char *value;
		} info[] = {
			{ "name",			name		},
			{ "description",	description	},
			{ "version",		version		},
			{ "bugreport",		bugreport	},
			{ "url",			url			},
		};

		for(auto &item : info) {
			if(!strcasecmp(key,item.name)) {
				if(gettext_package && *gettext_package) {
					value = dgettext(gettext_package,item.value);
				} else {
					value = item.value;
				}
				return true;
			}
		}

		return false;
	}


 }
