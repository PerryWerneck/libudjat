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

 #pragma once

 #include <config.h>
 #include <udjat/tools/quark.h>
 #include <udjat/factory.h>
 #include <unordered_map>
 #include <mutex>
 #include <udjat/request.h>

 using namespace std;

 namespace Udjat {

	class Factory::Controller {
	private:
		static recursive_mutex guard;

		// Hash method
		class Hash {
		public:
			inline size_t operator() (const char * str) const {
				// https://stackoverflow.com/questions/7666509/hash-function-for-string
				size_t value = 5381;

				for(const char *ptr = str; *ptr; ptr++) {
					value = ((value << 5) + value) + tolower(*ptr);
				}

				return value;
			}
		};

		// Equal method
		class Equal {
		public:
			inline bool operator() (const char *a, const char *b) const {
				return strcasecmp(a,b) == 0;
			}
		};

		/// @brief The list of active factories.
		std::unordered_map<const char *, const Factory *, Hash, Equal> factories;

		Controller() {
		}

	public:
		static Controller & getInstance();

		bool parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) const;
		bool parse(const char *name, Abstract::State &parent, const pugi::xml_node &node) const;

		const Factory * find(const char *name);

		void getInfo(Response &response) noexcept;

		void insert(const Factory *factory);
		void remove(const Factory *factory);

	};

 }

