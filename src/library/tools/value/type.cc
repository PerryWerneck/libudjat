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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <libintl.h>
 
 static const struct {
	Udjat::Value::Type type;
	const char *name;
 } typenames[] = {
	{ Udjat::Value::Type::Undefined,	N_("undefined") },	// Allways the first one
	{ Udjat::Value::Type::Array,		N_("array") 	},
	{ Udjat::Value::Type::Object,		N_("object") 	},
	{ Udjat::Value::Type::String,		N_("string") 	},
	{ Udjat::Value::Type::Timestamp,	N_("timestamp") },
	{ Udjat::Value::Type::Signed,		N_("signed") 	},
	{ Udjat::Value::Type::Unsigned,		N_("unsigned") 	},
	{ Udjat::Value::Type::Real,			N_("real") 		},
	{ Udjat::Value::Type::Boolean,		N_("boolean") 	},
	{ Udjat::Value::Type::Fraction,		N_("fraction") 	},
	{ Udjat::Value::Type::Icon,			N_("icon") 		},
	{ Udjat::Value::Type::Url,			N_("url") 		},
	{ Udjat::Value::Type::State,		N_("state") 	},

	{ Udjat::Value::Type::Signed,		N_("int") 		},
	{ Udjat::Value::Type::Signed,		N_("integer") 	},
	{ Udjat::Value::Type::Signed,		N_("number") 	},
 };

 namespace Udjat {

	Value::Type Value::TypeFactory(const XML::Node &node, const char *attrname, const char *def) {
		return Value::TypeFactory(Udjat::String{node,attrname,def}.c_str());
	}

	Value::Type Value::TypeFactory(const char *name) {

		for(size_t ix = 0; ix < N_ELEMENTS(typenames); ix++) {
			if(!strcasecmp(typenames[ix].name,name)) {
				return typenames[ix].type;
			}
		}

#ifdef GETTEXT_PACKAGE
		for(size_t ix = 0; ix < N_ELEMENTS(typenames); ix++) {
			if(!strcasecmp(dgettext(GETTEXT_PACKAGE,typenames[ix].name),name)) {
				return typenames[ix].type;
			}
		}
#endif

		Logger::String{"Unknown type '",name,"' assuming undefined"}.warning();

		return Value::Undefined;

	}

 }

 namespace std {

	UDJAT_API const char * to_string(Udjat::Value::Type type) noexcept {

		for(size_t ix = 0; ix < N_ELEMENTS(typenames); ix++) {
			if(typenames[ix].type == type) {
#ifdef GETTEXT_PACKAGE
				return dgettext(GETTEXT_PACKAGE,typenames[ix].name);
#else
				return typenames[ix].name;
#endif
			}
		}
		return _( "Unknown" );

	}

 } 
 