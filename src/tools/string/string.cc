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
 #include <udjat/tools/intl.h>
 #include <cstdarg>
 #include <udjat/tools/quark.h>

 using namespace std;

 namespace Udjat {

	String::String(const XML::Node &node, const char *attrname, const char *def) {

		auto attribute = node.attribute(attrname);

		if(attribute) {
			assign(attribute.as_string(def ? def : ""));
		} else if(def) {
			assign(def);
		} else {
			throw runtime_error(Logger::Message("Required attribute '{}' is missing",attrname));
		}

		if(empty()) {
			return;
		}

		expand(node);

	}

	String & String::concat(const bool value) {
		append(value ? _( "yes" ) : _( "no" ) );
		return *this;
	}

	String & String::strip() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::strip(ptr));
		free(ptr);
		return *this;
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
			const char *ptr = strcasestr(c_str(),xlat[ix].from);
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

	std::vector<String> String::split(const char *delim) {

		std::vector<String> strings;

		const char *ptr = c_str();
		while(ptr && *ptr) {
			const char *next = strstr(ptr,delim);
			if(!next) {
				strings.push_back(String(ptr).strip());
				break;
			}

			while(*next && isspace(*next))
				next++;

			strings.push_back(String(ptr,(size_t) (next-ptr)).strip());
			ptr = next+1;
			while(*ptr && isspace(*ptr)) {
				ptr++;
			}

		}

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

	size_t String::select(const char *value, ...) {

		size_t index = 0;

		va_list args;
		va_start(args, value);
		while(value) {

			if(!strcasecmp(c_str(),value)) {
				va_end(args);
				return index;
			}

			index++;
			value = va_arg(args, const char *);
		}
		va_end(args);

		return -1;

	}

	bool String::as_bool(bool def) {

		if(empty()) {
			return def;
		}

		if(!strcasecmp(c_str(),_("yes"))) {
			return true;
		}

		if(!strcasecmp(c_str(),_("no"))) {
			return false;
		}

		if(!strcasecmp(c_str(),_("true"))) {
			return true;
		}

		if(!strcasecmp(c_str(),_("false"))) {
			return false;
		}

		if(at(0) == 's' || at(0) == 'S' || at(0) == 't' || at(0) == 'T' || at(0) == '1') {
			return true;
		}

		if(at(0) == 'n' || at(0) == 'N' || at(0) == 'f' || at(0) == 'F' || at(0) == '0') {
			return false;
		}

		if(at(0) == '?' || !strcasecmp(c_str(),"default") || !strcasecmp(c_str(),_("default"))) {
			return def;
		}

		clog << "Unexpected boolean keyword '" << c_str() << "', assuming '" << (def ? "true" : "false") << "'" << endl;

		return def;
	}

	template<typename T>
	static T convert(const T value, const char *str) {

		while(*str && isspace(*str))
			str++;

		if(*str) {

			static const char * names[] = { "B", "KB", "GB", "MB", "TB" };
			T multiplier = 1;

			for(size_t ix = 0; ix < N_ELEMENTS(names);ix++) {

				if(!strcasecmp(str,names[ix])) {
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

		return convert<unsigned long long>(rc,str+bytes);

	}

	const char * String::as_quark() const {
		return Quark(*this).c_str();
	}


 }

