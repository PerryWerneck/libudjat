/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implement template pages.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/template.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <fcntl.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/variant.h>
 #include <stdexcept>
 #include <sstream>
 #include <udjat/tools/file/path.h>

 using namespace std;

 namespace Udjat {

	Template::Template(const char *name, const MimeType mimetype) {

		Config::Value<string> path{"theme","template-path"};

		if(!path.empty()) {
			filepath = path;
		} else {
#ifdef DEBUG
			filepath = String{(const char *) getenv("PWD"),"/templates"}.c_str();
#else
			filepath = Application::DataDir{"templates"};
#endif
		}

		filepath.append(name,".",std::to_string(mimetype,true));

		debug("Template file set to '",filepath.c_str(),"'");

	}

	// time_t Template::last_modified() const {

	// 	struct stat st;
	// 	if(stat(filename.c_str(), &st) < 0) {

	// 		Logger::String{filename.c_str(),": ",strerror(errno)}.error();
	// 		status.assign(HTTP::NotFound);
	// 		return send(status,false);

	// 	}

	// 	if(!S_ISREG(st.st_mode)) {
		
	// 		status.assign(HTTP::NotFound);
	// 		error(status.code,String{filename.c_str()," is not a regular file"}.c_str());
	// 		return send(status,false);

	// 	}

	// }

	static const char *get_default(const char *key) {

		static const struct {
			const char *key;
			const char *value;
		} defs[] = {
			{ "lang", 		N_("en") },
			{ "page-style",	"/css/style.css" },
			{ "page-title",	N_("Undefined page title") },
		};

		for(const auto &def : defs) {
			if(!strcasecmp(key,def.key)) {
				return dgettext(GETTEXT_PACKAGE,def.value);
			}
		}

		return "";
	}

	void Template::apply(std::ostream &stream, const std::function<bool(const char *key, std::ostream &stream)> &callback) {

		if(!filepath) {
			Logger::Message{"The file '{}' is unavailable within the selected theme",filepath.c_str()}.error();
			throw runtime_error(
				_("A required file is unavailable within the selected theme. Please contact the system administrator for assistance.")
			);
		}
		
		string text = filepath.load();

		// Build response.
		const char *ptr = text.c_str();

		while(*ptr) {

			const char *mark = strstr(ptr,marker);
			if(!mark) {
				stream << ptr;
				break;
			}

			size_t len = mark - ptr;
			stream.write(ptr,len);
			ptr += len;

			mark += 2;
			ptr = strstr(mark,"}");
			if(!ptr) {
				throw runtime_error("Malformed variable definition due to a missing '}' bracket");
			}
			std::string key{mark,(size_t) (ptr-mark)};
			ptr++;

			debug("---> key=",key.c_str());

			if(!callback(key.c_str(),stream)) {

				// Callback failed, try configuration file.
				std::string value = Config::Value<string>{"theme",key.c_str(),get_default(key.c_str())}.c_str();
				debug(key,"='",value.c_str(),"'");

				if(value.empty()) {
					// Cant get from configuration, try environment.
					const char *env = getenv(key.c_str());
					if(env) {
						value =env;
					}
				}
				
				if(value.empty()) {
					throw runtime_error(Logger::Message(
						_("Unable to resolve '{}' on template '{}'"),
						key.c_str(),filepath.c_str()
					));
				}

				stream << value.c_str();

			}

		}

	}

	void Template::apply(std::ostream &stream, const Value &value) {

		apply(stream, [&value](const char *key, std::ostream &stream){
			if(value.contains(key)) {
				stream << value[key];
				return true;
			}
			return false;
		});

	}

	void Template::apply(std::ostream &stream, const HTTP::Status &status) {

		apply(stream, [&status](const char *key, std::ostream &stream){

			if(!strcasecmp(key,"status-code")) {
				stream << ((int) status.code);
				return true;
			}

			if(!strcasecmp(key,"status-title")) {
				stream << status.title;
				return true;
			}

			if(!strcasecmp(key,"status-message")) {
				stream << status.message;
				return true;
			}

			if(!(strcasecmp(key,"detail") && strcasecmp(key,"status-detail") && strcasecmp(key,"status-body"))) {
				stream << status.detail;
				return true;
			}

			if(!strcasecmp(key,"status-domain")) {
				stream << status.domain;
				return true;
			}

			if(!strcasecmp(key,"status-url")) {
				stream << status.url;
				return true;
			}

			if(!strcasecmp(key,"status-category")) {
				stream << status.category;
				return true;
			}
		
			return false;
		});

	}

	std::string Template::to_string(const std::function<bool(const char *key, std::ostream &stream)> &callback) {
		stringstream stream;
		apply(stream,callback);
		return stream.str();
	}

 }
