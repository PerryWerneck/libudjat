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

 #include <udjat/tools/string.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/xml.h>
 #include <cstring>
 #include <ctype.h>
 #include <cstdlib>

 using namespace std;

 namespace Udjat {

	/// @brief Customized string expander.
	struct UDJAT_PRIVATE Expander {

		const std::function<bool(const char *key, std::string &value, bool dynamic, bool cleanup)> &method;

		constexpr Expander(const std::function<bool(const char *key, std::string &value, bool dynamic, bool cleanup)> &e) : method(e) {
		}

	};

	/// @brief List of registered string expanders.
	class UDJAT_PRIVATE Expanders : public std::vector<Expander> {
	public:
		static Expanders & getInstance() {
			static Expanders instance;
			return instance;
		}

		bool expand(const char *key, std::string &str, bool dynamic, bool cleanup) {

			for(Expander &expander : *this) {
				if(expander.method(key,str,dynamic,cleanup)) {
					return true;
				}
			}

			return false;
		}

	};

	void String::push_back(const std::function<bool(const char *key, std::string &value, bool dynamic, bool cleanup)> &method) {
		Expanders::getInstance().emplace_back(method);
	}

	String & String::strip() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::strip(ptr));
		free(ptr);
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

	static std::string getarguments(const std::string &key, const char *def) {

		const char *from = strchr(key.c_str(),'(');
		if(!from) {
			return def;
		}

		const char *to = strchr(++from,')');
		if(!to) {
			throw runtime_error(string{"Invalid expression '"} + key + "'");
		}

		if(to[1]) {
			clog << "string\tPossible misconfiguration in '" << key << "' expansion, found '" << (to+1) << "' after ')'" << endl;
		}

		return string(from,to-from);
	}

	String & String::expand(const Udjat::Abstract::Object &object, bool dynamic, bool cleanup) {
		return expand([&object,dynamic,cleanup](const char *key, std::string &str){
			if(object.getProperty(key,str))
				return true;
			return Expanders::getInstance().expand(key,str,dynamic,cleanup);
		},dynamic,cleanup);
	}

	String & String::expand(bool dynamic, bool cleanup) {
		return expand([dynamic,cleanup](const char *key, std::string &str){
			return Expanders::getInstance().expand(key,str,dynamic,cleanup);
		},dynamic,cleanup);
	}

	String & String::expand(const pugi::xml_node &node) {

		bool dynamic = node.attribute("expand-dynamic").as_bool(false);
		bool cleanup = node.attribute("clear-undefined").as_bool(false);

		return expand([node,dynamic,cleanup](const char *key, std::string &value){

			pugi::xml_attribute attribute;

			attribute = Object::getAttribute(node,key);
			if(attribute) {
				value = attribute.as_string();
				return true;
			}

			attribute = Object::getAttribute(node,key,false);
			if(attribute) {
				value = attribute.as_string();
				return true;
			}

			// Not expanded
			return Expanders::getInstance().expand(key,value,dynamic,cleanup);

		},
		dynamic,
		cleanup
		);

	}

	static string expandFromURL(const string &url) {

		try {

			auto lines = HTTP::Client(url).get().split("\n");
			if(!lines.empty()) {
				return *lines.begin();
			}

		} catch(const std::exception &e) {

			cerr << "string\tError '" << e.what() << "' expanding '" << url << "'" << endl;

		} catch(...) {

			cerr << "string\tUnexpected error expanding '" << url << "'" << endl;

		}

		return "";

	}


	String & String::expand(const std::function<bool(const char *key, std::string &str)> &expander, bool dynamic, bool cleanup) {

		auto from = find("${");
		while(from != string::npos) {

			auto to = find("}",from+3);
			if(to == string::npos) {
				throw runtime_error("Invalid ${} usage");
			}

			string value;
			string key(c_str()+from+2,(to-from)-2);

			//
			// First, try the supplied expand callback.
			//
			if(expander(key.c_str(),value)) {

				// Got value, apply it.
				replace(
					from,
					(to-from)+1,
					value.c_str()
				);

				from = find("${",from);
				continue;

			}

			//
			// If requested, expand dynamic values (timestamp, environment & url).
			//
			if(dynamic) {
				if(strncasecmp(key.c_str(),"timestamp",9) == 0) {
					replace(
						from,
						(to-from)+1,
						TimeStamp().to_string(getarguments(key,"%x %X")).c_str()
					);

					from = find("${",from);
					continue;
				}

				//
				// Search for URL identifier.
				//
				const char *sep = strstr(key.c_str(),"://");

				if(sep) {

					replace(
						from,
						(to-from)+1,
						expandFromURL(key)
					);
					from = find("${",from);
					continue;

				}

			}

			//
			// If cleanup is set, replace with an empty string, otherwise keep the ${} keyword.
			//
			if(cleanup) {

				//
				// Last resource, search the environment, if not available clean the value.
				//
#ifdef _WIN32
				// Windows, use the win32 api
				//const char *env = nullptr;
				char szEnvironment[4096];
				memset(szEnvironment,0,sizeof(szEnvironment));

				if(GetEnvironmentVariable(key.c_str(), szEnvironment, sizeof(szEnvironment)-1) != 0) {
					replace(
						from,
						(to-from)+1,
						szEnvironment
					);
				} else {
					replace(
						from,
						(to-from)+1,
						""
					);
				}

#else
				// Linux, use standard getenv.
				const char *env = getenv(key.c_str());
				if(env) {
					replace(
						from,
						(to-from)+1,
						env
					);
					from = find("${",from);
					continue;
				} else {
					replace(
						from,
						(to-from)+1,
						""
					);
				}
#endif // _WIN32

				from = find("${",from);

			} else {

				from = find("${",to+1);

			}

		}

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


 }

