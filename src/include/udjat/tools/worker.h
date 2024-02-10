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

	UDJAT_API bool exec(Request &request, Response::Value &response);
	UDJAT_API bool exec(Request &request, Response::Table &response);

	UDJAT_API bool introspect(Udjat::Value &value);

	class UDJAT_API Worker {
	private:
		const char * name = "";

	public:

		/// @brief Worker response type.
		enum ResponseType : uint8_t {
			None	= 0,	///< @brief The worker cant handle this request.
			Value	= 1,	///< @brief The worker response for this request is a value.
			Table	= 2,	///< @brief The worker response for this request is a table.
			Both	= 3,	///< @brief The worker response can be value, table or both.
		};

		static ResponseType ResponseTypeFactory(const char *name);
		static ResponseType ResponseTypeFactory(const XML::Node &node, const char *attrname = "response-type", const char *def = "None");

		class Controller;
		friend class Controller;

		Worker(const char *name, const ModuleInfo &info);

		Worker(const Quark &name, const ModuleInfo &info) : Worker(name.c_str(),info) {
		}

		static bool for_each(const std::function<bool(const Worker &worker)> &method);

		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,this->name) == 0;
		}

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;
		std::ostream & trace() const;

		/// @brief Find worker by path
		/// @param name Name of the required worker.
		/// @return The worker for path, exception if not found.
		static const Worker & find(const char *path);

		/// @brief Find worker by request
		/// @param name Name of the required worker.
		/// @return The worker for request, exception if not found.
		static const Worker & find(const Request &request);

		/// @brief Test if the request can run on this worker.
		/// @param request The request.
		/// @return The response type for this request.
		virtual ResponseType probe(const Request &request) const noexcept;

		/// @brief Get worker introspection.
		/// @param value The object for worker introspection.
		/// @return true if the value was updated.
		virtual bool introspect(Udjat::Value &value) const;

		/// @brief Get generic worker data, for example, the favicon.
		/// @param name The property name.
		/// @param value The response.
		/// @return true if the value was set.
		virtual bool getProperty(const char *name, Udjat::Value &property) const;

		virtual Udjat::Value & getProperties(Udjat::Value &properties) const;

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
		virtual bool head(Request &request, Abstract::Response &response) const;

		inline const char * c_str() const {
			return name;
		}

		virtual ~Worker();

		/// @brief Process request, get response as value.
		/// @return true if the response was updated.
		virtual bool work(Request &request, Response::Value &response) const;

		/// @brief Process request, get response as table.
		/// @return true if the response was updated.
		virtual bool work(Request &request, Response::Table &response) const;

	protected:

		/// @brief Information about the worker module.
		const ModuleInfo &module;

		ResponseType probe(const Request &request, ResponseType type) const noexcept;

	};

}
