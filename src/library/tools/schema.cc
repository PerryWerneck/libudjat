/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 #define LOG_DOMAIN "schema"

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/logger.h>
 #include <cstring>

 namespace Udjat {

	void Schema::add(const Item &item) {

		for(auto &itn : itens) {

			if(strcasecmp(item.name(),itn.name())) {
				// Not the same name, get next one.
				continue;
			}

			if(item.type() == itn.type()) {
				// Same name and type, ignore the new one.
				return;
			}

			// Type mismatch, change value type to string.
			Logger::String{"Duplicate item '",item.name(),"'"}.warning();
			itn.item_type = Schema::String;

		}

		itens.push_back(item);
	}

 }

 namespace std {

	const char * to_string(const Udjat::Schema::Type type) {

		static const struct {
			Udjat::Schema::Type type;
			const char *name;
		} tpnames[] = {
			{ Udjat::Schema::Type::String, 		"String"	},
			{ Udjat::Schema::Type::Timestamp,	"Timestamp"	},
			{ Udjat::Schema::Type::Signed,		"Signed"	},
			{ Udjat::Schema::Type::Unsigned,	"Unsigned"	},
			{ Udjat::Schema::Type::Double,		"Double"	},
			{ Udjat::Schema::Type::Float,		"Float"		},
			{ Udjat::Schema::Type::Boolean,		"Boolean"	},
			{ Udjat::Schema::Type::Icon,		"Icon"		},
			{ Udjat::Schema::Type::Url,			"Url"		},
			{ Udjat::Schema::Type::State,		"State"		},
			{ Udjat::Schema::Type::Percent,		"Percent"	},
		};

		for(const auto &tpname : tpnames) {
			if(tpname.type == type) {
				return tpname.name;
			}
		}

		return "unknown";
	}

 }


