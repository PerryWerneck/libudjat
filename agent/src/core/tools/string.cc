/*
 *
 * Copyright (C) <2019> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 *
 * @file
 *
 * @brief
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 *
 */

#ifdef _WIN32

	// Required for vasprint on MINGW
	#define _GNU_SOURCE
	#include <cstdio>

#endif // _WIN32

 #include <cstdarg>
 #include <udjat/string.h>
 #include <cstring>
 #include <cctype>

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	String & String::set(const char *fmt, ...) {
		va_list arg_ptr;
		va_start(arg_ptr, fmt);

		char * buffer = NULL;
		if(vasprintf(&buffer,fmt,arg_ptr) != -1) {
			std::string::assign(buffer);
			free(buffer);
		}

		va_end(arg_ptr);
		return *this;
	}

	String & String::set_va(const char *fmt, va_list va) {

		char * buffer = NULL;
		if(vasprintf(&buffer,fmt,va) != -1) {
			std::string::assign(buffer);
			free(buffer);
		}

		return *this;
	}

	String & String::setTimeStamp(std::time_t tm, const char *format) noexcept {
		char str[100];
		std::strftime( str, 100, format, std::localtime(&tm) );
		std::string::assign(str);
		return *this;
	}

	String & String::setTimeStamp(const std::tm &tm, const char *format) noexcept {
		char str[100];
		std::strftime( str, 100, format, &tm );
		std::string::assign(str);
		return *this;
	}

	bool String::hasSuffix(const char *str, const char *suffix) noexcept {

		int str_len     = strlen(str);
		int suffix_len  = strlen(suffix);

		if (str_len < suffix_len)
			return false;

		return strcasecmp( (str + str_len) - suffix_len, suffix) == 0;

	}

	String & String::capitalize() noexcept {

		if(!this->empty()) {
			(*this)[0] = toupper((*this)[0]);
		}

		if(this->size() > 1) {

			for(size_t ix = 1; ix < this->size();ix++) {
				(*this)[ix] = tolower((*this)[ix]);
			}

		}

		return *this;
	}


 }

