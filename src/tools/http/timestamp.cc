/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2015 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/http/timestamp.h>
 #include <cstdio>
 #include <ctype.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

 /*

	https://www.rfc-editor.org/rfc/rfc2616

	3.3 Date/Time Formats

	3.3.1 Full Date

	   HTTP applications have historically allowed three different formats
	   for the representation of date/time stamps:

		  Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
		  Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
		  Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

	   The first format is preferred as an Internet standard and represents
	   a fixed-length subset of that defined by RFC 1123 [8] (an update to
	   RFC 822 [9]). The second format is in common use, but is based on the
	   obsolete RFC 850 [12] date format and lacks a four-digit year.
	   HTTP/1.1 clients and servers that parse the date value MUST accept
	   all three formats (for compatibility with HTTP/1.0), though they MUST
	   only generate the RFC 1123 format for representing HTTP-date values
	   in header fields. See section 19.3 for further information.

		  Note: Recipients of date values are encouraged to be robust in
		  accepting date values that may have been sent by non-HTTP
		  applications, as is sometimes the case when retrieving or posting
		  messages via proxies/gateways to SMTP or NNTP.


 */

	static const char *month[]	= { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", nullptr };
	static const char *days[]	= { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", nullptr };

	std::string HTTP::TimeStamp::to_string() const noexcept {

		if(!value) {
			return "";
		}

		struct tm gmt;

		gmtime_r(&value,&gmt);

		char buffer[101];

		snprintf(
			buffer,
			100,
			"%3s, %02d %3s %02d %02d:%02d:%02d GMT",
				days[gmt.tm_wday],
				gmt.tm_mday,
				month[gmt.tm_mon],
				gmt.tm_year+1900,
				gmt.tm_hour,
				gmt.tm_min,
				gmt.tm_sec
		);

		return std::string(buffer);

	}

	static int getMonth(const char *name) {

		for(size_t m = 0; m < (sizeof(month)/sizeof(month[0]));m++) {
			if(!strcasecmp(month[m],name)) {
				return m;
			}
		}

		throw runtime_error(Logger::Message("Unexpected timestamp '{}'",name));
	}

	HTTP::TimeStamp & HTTP::TimeStamp::set(const char *str) {

		while(*str && !isdigit(*str))
			str++;

		if(*str) {

			struct tm gmt;
			char m[5];

			memset(&gmt,0,sizeof(gmt));

			if(sscanf(str,"%d %3s %d %d:%d:%d",&gmt.tm_mday,m,&gmt.tm_year,&gmt.tm_hour,&gmt.tm_min,&gmt.tm_sec) == 6) {

				gmt.tm_year -= 1900;
				gmt.tm_mon = getMonth(m);

				value = timegm(&gmt);

			} else {
				throw runtime_error(Logger::Message("Unexpected timestamp value '{}'",str));
			}

		} else {

			value = 0;

		}

		return *this;
	}

 }
