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
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/converters.h>
 #include <udjat/tools/intl.h>
 #include <cstdarg>
 #include <udjat/tools/quark.h>
 #include <sstream>
 #include <iomanip>

 using namespace std;

 namespace Udjat {

	String::String(const char **args, char delimiter) : std::string{args[0]} {

		char delim[] = {delimiter,0};

		for(size_t ix = 1; args[ix];ix++) {
			if(delimiter) {
				std::string::append(delim);
			}
			std::string::append(args[ix]);
		}

	}

	String::String(double value, int precision) {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(precision) << value;
		assign(stream.str());
	}

	String::String(float value, int precision) {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(precision) << value;
		assign(stream.str());
	}

	static const char * unit_names[] = { "B", "KB", "MB", "GB", "TB" };

	void String::append(const char *str) {
		if(str) {
			std::string::append(str);
		}
	}

	void String::append(const bool value) {
		append( (const char *) (value ? _( "yes" ) : _( "no" )) );
	}

	String & String::strip() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::strip(ptr));
		free(ptr);
		return *this;
	}

	char * String::strcasestr(const char *hs, const char *needle) {
#ifdef HAVE_STRCASESTR

		return ::strcasestr((char *) hs,needle);

#else
		std::string haystack{hs};
		std::string ndl{needle};

		for(char *ptr = (char *) haystack.c_str();*ptr;ptr++) {
			*ptr = toupper(*ptr);
		}

		for(char *ptr = (char *) ndl.c_str();*ptr;ptr++) {
			*ptr = toupper(*ptr);
		}

		auto pos = haystack.find(ndl);
		if(pos == std::string::npos) {
			return NULL;
		}

		return ((char *) hs)+pos;

#endif // HAVE_STRCASESTR
	}

	String & String::markup() {

		static const struct {
			const char *from;
			const char *to;
		} xlat[] = {
			{ "<b>", 	"\x1b[1m"	},
			{ "</b>",	"\x1b[0m"	}
		};

		for(size_t ix=0; ix < N_ELEMENTS(xlat); ix++) {
			const char *ptr = strcasestr(xlat[ix].from);
			if(ptr) {
				replace((ptr - c_str()),strlen(xlat[ix].from),xlat[ix].to);
			}
		}

		return *this;

	}

	String & String::chug() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::chug(ptr));
		free(ptr);
		return *this;
	}

	String & String::chomp() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::chomp(ptr));
		free(ptr);
		return *this;
	}

	UDJAT_API bool has_suffix(const char *str, const char *suffix, bool ignore_case) noexcept {

		if( !(str && *str) || !(suffix && *suffix)) {
			return false;
		}

		size_t str_len = strlen(str);
		size_t suffix_len = strlen (suffix);

		if (str_len < suffix_len)
			return false;

		if(ignore_case) {
			return strcasecmp(str + str_len - suffix_len, suffix) == 0;
		}

		return strcmp(str + str_len - suffix_len, suffix) == 0;

	}

	/// @brief Looks whether the string begins with prefix.
	UDJAT_API bool has_prefix(const char *str, const char *prefix, bool ignore_case) noexcept {

		if( !(str && *str) || !(prefix && *prefix)) {
			return false;
		}

		if(ignore_case) {
			return strncasecmp(str, prefix, strlen (prefix)) == 0;
		}

		return strncmp(str, prefix, strlen (prefix)) == 0;
	}

	bool String::for_each(const char *ptr, const char *delim, const std::function<bool(const String &value)> &func) {

		size_t szdelim = strlen(delim);

		while(ptr && *ptr) {

			const char *next = strstr(ptr,delim);
			if(!next) {
				return func(String{ptr}.strip());
			}
			next += szdelim;

			while(*next && isspace(*next))
				next++;

			if(func(String{ptr,(size_t) ((next-ptr)-1)}.strip())) {
				return true;
			}

			ptr = next;
			while(*ptr && isspace(*ptr)) {
				ptr++;
			}

		}

		return false;

	}

	bool String::for_each(const char *delim, const std::function<bool(const String &value)> &func) const {
		return for_each(c_str(),delim,func);
	}

	std::vector<String> String::split(const char *delim) const {

		std::vector<String> strings;

		for_each(c_str(),delim,[&strings](const String &value){
			strings.emplace_back(value.c_str());
			return false;
		});

		return strings;

	}

 	char * chomp(char *str) noexcept {

		size_t len = strlen(str);

		while(len--) {

			if(isspace(str[len])) {
				str[len] = 0;
			} else {
				break;
			}
		}

		return str;

	}

	char * chug (char *str) noexcept {

		char *start;

		for (start = (char*) str; *start && isspace(*start); start++);

		memmove(str, start, strlen ((char *) start) + 1);

		return str;
	}

	char * strip(char *str) noexcept {
		return chomp(chug(str));
	}

	UDJAT_API bool isnumber(const char *str) {
		for(const char *ptr = str;*ptr;ptr++) {
			if(!::isdigit(*ptr)) {
				return false;
			}
		}
		return true;
	}


	std::string & strip(std::string &str) noexcept {
		char *buffer = new char[str.size()+1];
		memcpy(buffer,str.c_str(),str.size());
		buffer[str.size()] = 0;
		strip(buffer);
		str.assign(buffer);
		delete[] buffer;
		return str;
	}

	std::string markup(const char *text) {
		return String{text}.markup();
	}

	std::string UDJAT_API strip(const char *str, ssize_t length) {

		if(length < 0) {
			length = strlen(str);
		}

		char *buffer = new char[length+1];
		memcpy(buffer,str,length);
		buffer[length] = 0;
		strip(buffer);

		std::string rc(buffer);
		delete[] buffer;

		return rc;
	}

	int String::select(const char *value, va_list args) const noexcept {

		if(empty()) {
			return -(errno = ENODATA);
		}

		int index = 0;
		while(value) {

			if(!strcasecmp(c_str(),value)) {
				va_end(args);
				return index;
			}

			index++;
			value = va_arg(args, const char *);
		}
		return -(errno = ENOENT);
	}

	int String::select(const char *value, ...) const noexcept {

		va_list args;
		va_start(args, value);
		int rc = select(value,args);
		va_end(args);

		return rc;

	}

	bool String::as_bool(bool def) {
		if(empty()) {
			return def;
		}
		return from_string<bool>(c_str());
	}

	template<typename T>
	inline string byte_to_string(T value, int precision) {

		T multiplier{1024};
		T selected{1};

		const char *name = unit_names[0];
		for(size_t ix = 1; ix < N_ELEMENTS(unit_names);ix++) {

			if(value >= multiplier) {
				selected = multiplier;
				name = unit_names[ix];
			}

			multiplier *= 1024.0D;

		}

		std::stringstream stream;

		stream
			<< std::fixed << std::setprecision(precision) << (((double) value)/selected);

		if(name && *name) {
			stream << " " << name;
		}

		return stream.str();

	}

	String & String::set_byte(double value, int precision) {
		assign(byte_to_string<double>(value,precision));
		return *this;
	}

	String & String::set_byte(unsigned long long value, int precision) {
		assign(byte_to_string<unsigned long long>(value,precision));
		return *this;
	}

	template<typename T>
	static T string_to_byte(const T value, const char *str) {

		while(*str && isspace(*str))
			str++;

		if(*str) {

			T multiplier = 1;

			for(size_t ix = 0; ix < N_ELEMENTS(unit_names);ix++) {

				if(!strcasecmp(str,unit_names[ix])) {
					return multiplier * value;
				}

				multiplier *= 1024;

			}

		}

		return value;

	}

	unsigned long long String::as_ull() const {

		if(empty()) {
			return 0;
		}

		const char *str = c_str();
		size_t bytes = 0;

		unsigned long long rc = stoll(str,&bytes);
		str += bytes;

		return string_to_byte<unsigned long long>(rc,str+bytes);

	}

	const char * String::as_quark() const {
		return Quark(*this).c_str();
	}


 }

