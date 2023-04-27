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
#include <private/misc.h>
#include <cstring>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/intl.h>
#include <iostream>
#include <time.h>

using namespace std;

namespace Udjat {

	TimeStamp::TimeStamp(const char *time, const char *format) : value{parse(time,format)} {
	}

	std::string TimeStamp::to_string(const char *format) const noexcept {

		if(!value)
			return "";

		char timestamp[80];
		memset(timestamp,0,80);

		struct tm tm;

#if defined(HAVE_LOCALTIME_R)

		localtime_r(&value,&tm);

#elif defined(_WIN32)

		localtime_s(&tm,&value);

#else

		tm = *localtime(&value);

#endif // HAVE_LOCALTIME_R

		size_t len = strftime(timestamp, 79, format, &tm);
		if(len == 0) {
			return "";
		}

		return std::string(timestamp,len);
	}

	std::string TimeStamp::to_verbose_string() const noexcept {

		if(!value)
			return "";

		if(value < 60) {
			return _( "Less than one minute" );
		}

		long days = value / 86400;
		long hours = (value - (days * 86400)) / 3600;
		long mins = (value - (days * 86400) - (hours * 3600)) / 60;

		uint8_t key = (days > 0 ? 1 : 0) + (hours > 0 ? 2 : 0) + (mins > 0 ? 4 : 0);

		static const struct {
			uint8_t	key;
			const char * fmt;
		} itens[] = {
			{ 1,	N_( "{d} {D}" ) 						},	// Only days.
			{ 2,	N_( "{h} {H}" )							},	// Only hours.
			{ 3,	N_( "{d} {D} and {h} {H}" )				},	// Days and hours.
			{ 4,	N_( "{m} {M}" )							},	// Only minutes.
			{ 5,	N_( "{d} {D} and {m} {M}" )				},	// Days and minutes.
			{ 6,	N_( "{h} {H} and {m} {M}" )				},	// Hours and minutes.
			{ 7,	N_( "{d} {D}, {h} {H} and {m} {M}" )	},	// Days, hours and minutes.
		};

		struct {
			const char *tag;
			string text;
		} values[] = {
			{ "{d}", std::to_string(days) },
			{ "{D}", string( days > 1 ?  _("days") : _("day")) },
			{ "{h}", std::to_string(hours) },
			{ "{H}", string( hours > 1 ? _("hours") : _("hour")) },
			{ "{m}", std::to_string(mins) },
			{ "{M}", string( mins > 1 ? _("minutes") : _("minute")) }
		};

		debug("days: ",days," hours: ",hours," minutes: ",mins," key: ",((int) key));

		string out;
		for(size_t item = 0; item < (sizeof(itens)/sizeof(itens[0]));item++) {

			if(itens[item].key == key) {

#ifdef GETTEXT_PACKAGE
				out = dgettext(GETTEXT_PACKAGE,itens[item].fmt);
#else
				out = itens[item].fmt;
#endif // GETTEXT_PACKAGE

				for(size_t value = 0; value < (sizeof(values)/sizeof(values[0])); value++) {
					size_t pos = out.find(values[value].tag);
					if(pos != string::npos) {
						out.replace(pos,strlen(values[value].tag),values[value].text);
					}
				}

				break;
			}

		}

		return out;

	}

	TimeStamp & TimeStamp::set(const char *time, const char *format) {
		this->value = parse(time,format);
		return *this;
	}

	time_t TimeStamp::parse(const char *time, const char *format) {

		if(!strcasecmp(time,"now")) {
			return ::time(0);
		}

		struct tm t;
		memset(&t,0,sizeof(t));

		if(format && *format) {

			// Have format, use-it
			if(!strptime(time, format, &t)) {
				throw runtime_error(string{"Can't parse '"} + time + "' in the requested format");
			}

			return mktime(&t);

		} else {

			// Dont have format, try strptime known ones.
			static const char *formats[] = {
				"%Y-%m-%d %T",
				"%y-%m-%d %T",
				"%x %X",
			};

			for(size_t ix = 0; ix < (sizeof(formats)/sizeof(formats[0])); ix++) {
				if(strptime(time,formats[ix],&t)) {
					return mktime(&t);
				}
			}

			// Can't find using strptime, try another way
			static const struct {
				time_t value;
				const char *singular;
				const char *plural;

				inline size_t compare(const char *str) const noexcept {

					size_t length;

					length = strlen(plural);
					if(!strncasecmp(str,plural,length)) {
						return length;
					}

					// Check value.
					length = strlen(singular);
					if(!strncasecmp(str,singular,length)) {
						return length;
					}

					// Check translations.
					#ifdef GETTEXT_PACKAGE
						const char *translated = dgettext(GETTEXT_PACKAGE,plural);

						length = strlen(translated);
						if(!strncasecmp(str,translated,length)) {
							return length;
						}

						translated = dgettext(GETTEXT_PACKAGE,singular);
						length = strlen(translated);
						if(!strncasecmp(str,translated,length)) {
							return length;
						}

					#endif // GETTEXT_PACKAGE
					return 0;
				}

			} values[] = {

				{ 1,		N_("second"),	N_("seconds")	},
				{ 60,		N_("minute"),	N_("minutes")	},
				{ 3600,		N_("hour"),		N_("hours")		},
				{ 86400,	N_("day"),		N_("days")		},

			};

			time_t value = 0;
			const char *ptr = time;

			debug("Converting ",ptr);
			while(*ptr) {

				while(*ptr && (ispunct(*ptr) || isspace(*ptr))) {
					ptr++;
				}

				time_t v = 0;
				while(*ptr && isdigit(*ptr)) {
					v *= 10;
					v += (*ptr - '0');
					ptr++;
				}

				while(*ptr && (ispunct(*ptr) || isspace(*ptr))) {
					ptr++;
				}

				if(*ptr) {
					size_t len = 0;
					for(size_t ix=0; ix < N_ELEMENTS(values);ix++) {
						len = values[ix].compare(ptr);
						if(len) {
							ptr += len;
							debug(v," ",string{ptr,len}.c_str(),"=",(v * values[ix].value));
							v *= values[ix].value;
							break;
						}
					}

					if(!len) {
						throw runtime_error(Logger::Message{"Dont know how to translate {}",time});
					}
				}

				value += v;

			}

			return value;
		}

		throw runtime_error(Logger::Message{"Can't parse '{}' in any known format",time});

	}

}
