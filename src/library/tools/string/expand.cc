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
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/object.h>

 #ifdef _WIN32
	#include <private/win32.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	/// @brief Customized string expander.
	struct UDJAT_PRIVATE Expander {

		const std::function<bool(const char *key, std::string &value, bool dynamic, bool cleanup)> method;

		Expander(const std::function<bool(const char *key, std::string &value, bool dynamic, bool cleanup)> e) : method(e) {
		}

	};

	/// @brief List of registered string expanders.
	class UDJAT_PRIVATE Expanders : public std::vector<Expander> {
	public:
		static Expanders & getInstance() {
			static Expanders instance;
			return instance;
		}

		/// @brief Expand 'key' in the string.
		/// @param key The key to expand.
		/// @param str The string to expand.
		/// @return true if the string was expanded.
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

	String & String::expand(char marker, const Udjat::Abstract::Object &object, bool dynamic, bool cleanup) {
		return expand(marker,[&object,dynamic,cleanup](const char *key, std::string &str){
			if(object.getProperty(key,str))
				return true;
			return false;
		},dynamic,cleanup);
	}

	String & String::expand(const Udjat::Abstract::Object &object, bool dynamic, bool cleanup) {
		return expand('$',object,dynamic,cleanup);
	}

	String & String::expand(bool dynamic, bool cleanup) {
		return expand('$',dynamic,cleanup);
	}

	String & String::expand(char marker, bool dynamic, bool cleanup) {
		return expand(marker,[dynamic,cleanup](const char *key, std::string &str){
			return false;
		},dynamic,cleanup);
	}

	static bool check_node(XML::Node &xml, const char *key, std::string &value) {

		for(auto child = xml.child("attribute"); child; child = child.next_sibling("attribute")) {

			// Check the attribute name.
			if(!strcasecmp(key,child.attribute("name").as_string("*"))) {

				if(is_allowed(child)) {
					value = child.attribute("value").as_string();
					return true;
				}

			}
		}

		return false;
	}

	String & String::expand(const XML::Node &node, const char *group) {
		return expand('$',node,group);
	}

	String & String::expand(char mrk, const XML::Node &node, const char *group) {

		bool dynamic = node.attribute("expand-dynamic").as_bool(false);
		bool cleanup = node.attribute("clear-undefined").as_bool(false);

		char defmarker[2] = {mrk,'\0'};
		const char *marker = node.attribute("variable-marker").as_string(defmarker);

		if(!(marker && *marker)) {
			throw system_error(EINVAL,system_category(),"Required attribute 'variable-marker' is empty or invalid");
		}

		group = node.attribute("settings-from").as_string(group);

		return expand(marker[0],[node,dynamic,cleanup,group](const char *key, std::string &value) {

			// Check node attributes
			{
				pugi::xml_attribute attribute = node.attribute(key);
				if(attribute) {
					value = attribute.as_string();
					return true;
				}
			}

			// Search the XML tree for an attribute with the required name.
			for(XML::Node xml = node;xml;xml = xml.parent()) {

				if(is_allowed(xml)) {

					// Search attributes.
					if(check_node(xml,key,value)) {
						return true;
					}

					// Search attribute lists.
					for(auto lst = xml.child("attribute-list"); lst; lst = lst.next_sibling("attribute-list")) {

						if(is_allowed(lst) && check_node(lst,key,value)) {
							return true;
						}

					}

				}

			}

			if(group && Config::hasKey(group,key)) {
				// Get from the configuration file.
				value = Config::get(group, key, "");
				return true;
			}

			// Not expanded, use non xml expanders.
			return Expanders::getInstance().expand(key,value,dynamic,cleanup);

		},
		dynamic,
		cleanup
		);

	}

	static string expandFromURL(const string &url) {

		try {

			auto lines = URL{url}.get().split("\n");
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
		return expand('$',expander,dynamic,cleanup);
	}

	static bool expand_value(const char *key, std::string &value, bool dynamic, bool cleanup) {

		// Check the attribute name.
		if(!strcasecmp(key,"logdir")) {
			value = Application::LogDir();
			return true;
		}

		if(!strcasecmp(key,"datadir")) {
			value = Application::DataDir();
			return true;
		}

		if(!strcasecmp(key,"cachedir")) {
			value = Application::CacheDir();
			return true;
		}

		if(!strcasecmp(key,"appname")) {
			value = Application::Name();
			return true;
		}

		//
		// If requested, expand dynamic values (timestamp, environment & url).
		//
		if(dynamic) {

			if(strncasecmp(key,"timestamp",9) == 0) {
				value = TimeStamp().to_string(getarguments(key,"%x %X")).c_str();
				return true;
			}

			const char *sep = strstr(key,"://");
			if(sep) {
				value = expandFromURL(key);
				return true;
			}

		}

#ifdef _WIN32
		if(!strcasecmp(key,"home")) {
			value = Win32::KnownFolder{FOLDERID_Profile};
			return true;
		}
#endif // _WIN32

		//
		// Apply custom expanders.
		//
		if(Expanders::getInstance().expand(key,value,dynamic,cleanup)) {
			return true;
		}

		//
		// If cleanup is set, replace with an empty string, otherwise keep the marker.
		//
		if(cleanup) {

			//
			// Last resource, search the environment, if not available clean the value.
			//

#ifdef _WIN32
			// Windows, use the win32 api
			char szEnvironment[4096];
			memset(szEnvironment,0,sizeof(szEnvironment));

			if(GetEnvironmentVariable(key, szEnvironment, sizeof(szEnvironment)-1) != 0) {
				value = szEnvironment;
				return true;
			}

#else

			// Linux, use standard getenv.
			const char *env = getenv(key);
			if(env) {
				value = env;
				return true;
			}
#endif // _WIN32

			value = "";
			return true;

		}

		return false;
	}

	String & String::expand(char marker, const std::function<bool(const char *key, std::string &str)> &expander, bool dynamic, bool cleanup) {

		char starter[3] = { marker, '{', 0 };

		auto from = find(starter);
		while(from != string::npos) {

			auto to = find("}",from+3);
			if(to == string::npos) {
				throw runtime_error(Logger::String{"Invalid use of '",starter,"}' on '",c_str(),"' at ",from});
			}

			string key{c_str()+from+2,(to-from)-2};
			size_t split[] = {0,0};

			size_t ix = key.find('[');
			if(ix != string::npos) {
				ix++;
				size_t iy = key.find(']',ix);
				if(iy == string::npos) {
					throw runtime_error(Logger::String{"Invalid use of '[' in '",key,"'"});
				}

				auto vals = String{key.c_str()+ix,iy-ix}.split(":");
				if(vals.size() < 2) {
					throw runtime_error(Logger::String{"Invalid use of '[' in '",key,"'"});
				}
			
				if(!(vals[0].isnumber() && vals[1].isnumber())) {
					throw runtime_error(Logger::String{"Invalid use of '[' in '",key,"': non numeric values"});
				}

				split[0] = atoi(vals[0].c_str());
				split[1] = atoi(vals[1].c_str());

				if(split[0] == split[1]) {
					throw runtime_error(Logger::String{"Invalid use of '[' in '",key,"': invalid range"});
				}

				key = key.substr(0,ix-1);

			}
			
			string value;
			if(expander(key.c_str(),value) || expand_value(key.c_str(),value,dynamic,cleanup)) {

				if(split[1] > split[0]) {
					value = value.substr(split[0],split[1]-split[0]);
				}

				replace(
					from,
					(to-from)+1,
					value.c_str()
				);

				from = find(starter,from);

			} else {

				from = find(starter,to+1);

			}
					
		}

		return *this;
	}


 }

