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
#include <iostream>

using namespace std;

namespace Udjat {

	std::string TimeStamp::to_string(const char *format) const noexcept {

		if(!value)
			return "";

		char timestamp[80];
		memset(timestamp,0,80);

		struct tm tm;

#ifdef HAVE_LOCALTIME_R
		localtime_r(&value,&tm);
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

		struct tm t;
		memset(&t,0,sizeof(t));

		if(format && *format) {

			// Have format, use-it
			if(!strptime(time, format, &t)) {
				throw runtime_error(string{"Can't parse '"} + time + "' in the requested format");
			}

		} else {

			// Dont have format, try known ones.
			static const char *formats[] = {
				"%Y-%m-%d %T",
				"%y-%m-%d %T",
				"%x %X",
			};

			bool found = false;
			for(size_t ix = 0; ix < (sizeof(formats)/sizeof(formats[0])); ix++) {
				if(strptime(time,formats[ix],&t)) {
					found = true;
					break;
				}
			}

			if(!found) {
				throw runtime_error(string{"Can't parse '"} + time + "' in any known format");
			}

		}

#ifdef DEBUG
		cout << "day=" << t.tm_mday << " month=" << t.tm_mon << " Year=" << t.tm_year << endl;
#endif // DEBUG

		this->value = mktime(&t);

		return *this;
	}

}
