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
 #include <udjat/tools/factory.h>
 #include <list>
 #include <mutex>
 #include <udjat/tools/request.h>

 using namespace std;

 namespace Udjat {

	class Factory::Controller {
	private:
		static recursive_mutex guard;

		/// @brief The list of active factories.
		std::list<Factory *> factories;

		Controller() {
		}

	public:
		static Controller & getInstance();

		/// @brief Find factory by name.
		/// @param name Factory name.
		/// @return The requested factory or nullptr.
		Factory * find(const char *name);

		void insert(Factory *factory);
		void remove(Factory *factory);

		bool for_each(const std::function<bool(Factory &factory)> &func);
		bool for_each(const char *name, const std::function<bool(Factory &factory)> &func);
		bool for_each(const XML::Node &node, const std::function<bool(Factory &factory)> &func);

	};

 }

