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
 #include <udjat/tools/value.h>
 #include <stdexcept>

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

		filepath.append(
			"/",name
		);

		if(mimetype != MimeType::none) {
			filepath.append(
				".",std::to_string(mimetype,true)
			);
		}

		debug("Template file set to '",filepath.c_str(),"'");

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

			if(!callback(key.c_str(),stream)) {

				// Callback failed, fallback to configuration file.
				Config::Value<string> value{"theme",key.c_str()};
				if(!value.empty()) {
					stream << value.c_str();
				} else {
					throw runtime_error(Logger::Message(
						_("Unable to resolve '{}' on template '{}'"),
						key.c_str(),filepath.c_str()
					));
				}

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

			if(!strcasecmp(key,"status-body")) {
				stream << status.body;
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


 }
