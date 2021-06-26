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

#include <udjat/defs.h>
#include <udjat/request.h>
#include <udjat/tools/quark.h>

namespace Udjat {

	class UDJAT_API Worker {
	private:
		const char * name;
		class Controller;
		friend class Controller;

	protected:

		/// @brief Information about the worker module.
		const ModuleInfo *info;

	public:
		Worker(const char *name, const ModuleInfo *info);
		Worker(const char *name);

		Worker(const Quark &name, const ModuleInfo *info) : Worker(name.c_str(),info) {
		}

		Worker(const Quark &name) : Worker(name.c_str()) {
		}

		/// @brief Execute request, update response
		/// @return false if the request method was not allowed.
		static bool work(const char *name, Request &request, Response &response);

		/// @brief Get array with information about all registered worker.
		static void getInfo(Response &response);

		/// @brief Get module information.
		inline const ModuleInfo * getModuleInfo() const noexcept {
			return this->info;
		}

		/// @brief Get Worker by name.
		static const Worker * find(const char *name);

		/// @brief Process only the 'get' method.
		/// @return false if the request method was not allowed.
		virtual bool get(Request &request, Response &response) const;

		/// @brief Process only the 'head' method.
		/// @return false if the request method was not allowed.
		virtual bool head(Request &request, Response &response) const;

		size_t hash() const;

		inline const char * c_str() const {
			return name;
		}

		virtual ~Worker();

		/// @brief Process all methods.
		/// @return true if the method was allowed.
		virtual bool work(Request &request, Response &response) const;

	};

}
