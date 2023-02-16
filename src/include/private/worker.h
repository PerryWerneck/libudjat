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
 #include <udjat/defs.h>
 #include <udjat/worker.h>
 #include <udjat/request.h>
 #include <mutex>
 #include <unordered_map>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	class Worker::Controller {
	private:

		static std::recursive_mutex guard;

		Controller();
		~Controller();

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

		std::unordered_map<const char *, const Worker *, Hash, Equal> workers;

	public:
		static Controller & getInstance();

		void insert(const Worker *worker);
		void remove(const Worker *worker);

		const Worker * find(const char *name) const;

		bool for_each(const std::function<bool(const Worker &worker)> &func);

	};

 }

