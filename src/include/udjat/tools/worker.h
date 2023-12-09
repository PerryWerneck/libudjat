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
#include <udjat/tools/request.h>
#include <udjat/tools/response.h>
#include <udjat/tools/report.h>
#include <udjat/tools/quark.h>
#include <ostream>

namespace Udjat {

	class UDJAT_API Worker {
	private:
		const char * name = "";
		class Controller;
		friend class Controller;

	protected:

		/// @brief Information about the worker module.
		const ModuleInfo &module;

	public:
		Worker(const char *name, const ModuleInfo &info);

		Worker(const Quark &name, const ModuleInfo &info) : Worker(name.c_str(),info) {
		}

		static bool for_each(const std::function<bool(const Worker &worker)> &method);

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;
		std::ostream & trace() const;

		/// @brief Find worker by name
		/// @param name Name of the required worker.
		/// @return The worker for path, exception if not found.
		static const Worker * find(const char *path);

		/// @brief Test if the request can run on this worker.
		/// @param request The request.
		/// @param response The response.
		/// @return True if the request was fullfiled, false if not.
		virtual bool exec(Request &request, Response::Value &response) const;
		virtual bool exec(Request &request, Response::Table &response) const;

		/// @brief Get worker introspection.
		/// @param value The object for worker introspection.
		/// @return true if the value was updated.
		virtual bool introspect(Udjat::Value &value) const;

		virtual Value & getProperties(Value &properties) const;

		/// @brief Get module information.
		inline const ModuleInfo & getModuleInfo() const noexcept {
			return this->module;
		}

		/// @brief Process only the 'get' method.
		/// @return false if the request method was not allowed.
		virtual bool get(Request &request, Response::Value &response) const;
		virtual bool get(Request &request, Response::Table &response) const;

		/// @brief Process only the 'head' method.
		/// @return false if the request method was not allowed.
		virtual bool head(Request &request, Response::Value &response) const;

		inline const char * c_str() const {
			return name;
		}

		virtual ~Worker();

		/// @brief Process all methods.
		/// @return true if the method was allowed.
		virtual bool work(Request &request, Response::Value &response) const;

		/// @brief Process report method.
		virtual bool work(Request &request, Response::Table &response) const;

	};

}
