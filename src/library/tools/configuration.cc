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
 #include <private/configuration.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/logger.h>
 #include <cstring>
 #include <ctype.h>

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	namespace Config {

		Value<std::vector<std::string>>::Value(const char *group, const char *name, const char *def, const char *delim) {

			// https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
			// https://www.codespeedy.com/multiple-ways-to-split-a-string-in-cpp/
			string str = get(group, name, def);

			const char *ptr = str.c_str();
			while(ptr && *ptr) {
				const char *next = strstr(ptr,delim);
				if(!next) {
					emplace_back(ptr);
					break;
				}

				while(*next && isspace(*next))
					next++;

				if(*next) {
					emplace_back(ptr,(size_t) (next-ptr));
					ptr = next+1;
					while(*ptr && isspace(*ptr)) {
						ptr++;
					}
				}

			}

		}

		Value<std::string> & Value<std::string>::set(const char *name, const char *value) {
			string key{"${"};
			key += name;
			key += "}";
			auto pos = find(key);
			if(pos != string::npos) {
				replace(pos,key.size(),value);
			}
			return *this;
		}

		Value<std::string> & Value<std::string>::set(const char *name, const char *group, const char *key, const char *def) {
			return set(name, Value<std::string>(group,key,def).c_str());
		}

		bool call(const char *group, const std::function<bool(const Properties &properties)> &method) {

			class Properties : public Udjat::Properties {
				private:
					const char *group;

				public:
					constexpr Properties(const char *g) : group{g} {
					}

					bool contains(const char *key) const noexcept override {
						return Controller::getInstance().contains(group,key);
					}

					const String get(const char *attrname, const char *def) const override {
						return Controller::getInstance().get(group,attrname,def);
					}

					bool get(const char *attrname, const bool def) const override {
						return Controller::getInstance().get(group,attrname,def);
					}

					double get(const char *attrname, const double def) const override {
						return Controller::getInstance().get(group,attrname,def);
					}

					float get(const char *attrname, const float def) const override {
						return Controller::getInstance().get(group,attrname,def);
					}

					int get(const char *attrname, const int def) const override {
						return Controller::getInstance().get(group,attrname,def);
					}

					unsigned int get(const char *attrname, const unsigned int def) const override {
						return Controller::getInstance().get(group,attrname,def);
					}

			};

			return method(Properties{group});

		}

	}

 }



