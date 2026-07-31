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
 #include <udjat/tools/variant.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <libintl.h>
 
 static const struct {
	Udjat::Variant::Type type;
	const char *name;
 } typenames[] = {
	{ Udjat::Variant::Type::Undefined,	N_("undefined") },	// Allways the first one
	{ Udjat::Variant::Type::Array,		N_("array") 	},
	{ Udjat::Variant::Type::Object,		N_("object") 	},
	{ Udjat::Variant::Type::String,		N_("string") 	},
	{ Udjat::Variant::Type::Timestamp,	N_("timestamp") },
	{ Udjat::Variant::Type::Signed,		N_("signed") 	},
	{ Udjat::Variant::Type::Unsigned,		N_("unsigned") 	},
	{ Udjat::Variant::Type::Real,			N_("real") 		},
	{ Udjat::Variant::Type::Boolean,		N_("boolean") 	},
	{ Udjat::Variant::Type::Fraction,		N_("fraction") 	},
	{ Udjat::Variant::Type::Icon,			N_("icon") 		},
	{ Udjat::Variant::Type::Url,			N_("url") 		},
	{ Udjat::Variant::Type::State,		N_("state") 	},

	{ Udjat::Variant::Type::Signed,		N_("int") 		},
	{ Udjat::Variant::Type::Signed,		N_("integer") 	},
	{ Udjat::Variant::Type::Signed,		N_("number") 	},
 };

 namespace Udjat {

	Variant::Type Variant::TypeFactory(const Udjat::Properties &props, const char *attrname, const char *def) {
		return Variant::TypeFactory(props.get(attrname,def).c_str());
	}

	Variant::Type Variant::TypeFactory(const char *name) {

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

		return Variant::Undefined;

	}

 }

 namespace std {

	UDJAT_API const char * to_string(Udjat::Variant::Type type) {

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
 