/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declares a method call.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/abstract/object.h>
 #include <vector>
 #include <memory>

 namespace Udjat {

	/// @brief Generic API method.
	class UDJAT_API Method {
	private:

		/// @brief The method name.
		const char *_name;
		class Controller;

	protected:

		typedef Method Super;

		/// @brief Copy output values from single object to value.
		/// @param object The source object
		/// @param value The destination object, will receive the output properties from object.
		void success(const Abstract::Object &object, Udjat::Value &value) const;
		
	public:
		Method(const char *name);
		Method(const XML::Node &node);
		virtual ~Method();

		static Method & find(const char *name);

		inline const char * name() const noexcept {
			return _name;
		}

#if __cplusplus >= 202002L
		auto operator <=>(const char *n) const noexcept {
			return strcasecmp(n,this->_name);
		}
#endif

		/// @brief Get object introspections.
		/// @param input The value to receive method inputs.
		/// @param output The value to receive method outputs.
		virtual void introspect(Value &input, Value &output);

		/// @brief Enum method properties.
		/// @param call The callback to handle property, returns true to interrupt the loop.
		/// @return true if the loop was interrupted
		virtual bool for_each(const std::function<bool(const size_t index, bool input, const char *name, const Value::Type type)> &call) const = 0;

		/// @brief Execute method.
		/// @param path The path for object request.
		/// @param values The in/out values.
		virtual void call(const char *path, Udjat::Value &values) = 0;

		/// @brief Execute method.
		/// @param request The client request.
		/// @param response The response.
		virtual void call(Request &request, Response &response);

		/// @brief Execute method.
		/// @param path The path for object request.
		/// @param values The in/out values.
		static void call(const char *name, const char *path, Udjat::Value &values);

		/// @brief Execute method.
		/// @param request The client request.
		/// @param response The response.
		static void call(const char *name, Request &request, Response &response);

	};

	/*
	/// @brief XML defined methods.
	///
	/// Send output in jsend format (https://github.com/omniti-labs/jsend)
	///
	class UDJAT_API Method {
	public:

		/// @brief The API call worker method.
		struct Worker {

			/// @brief Execute script, update value.
			/// @param value The values for the worker.
			virtual void call(Udjat::Value &value) = 0;

		};

	private:

		struct Argument {
			const char *name = nullptr;
			const Value::Type type = Value::Undefined;
			bool input = true;

			constexpr Argument(const char *n, Value::Type t, bool i)
				: name{n}, type{t}, input{i} {
			}
			
			Argument(const XML::Node &node, bool input);
		};

		std::vector<Argument> args;

		std::vector<std::shared_ptr<Worker>> workers;

	protected:

		typedef Method Super;

		/// @brief Run workers, update values.
		/// @param values The set of value to read/update.
		void call(Udjat::Value &values);

	public:
		Method(const XML::Node &node);
		virtual ~Method();
	
		size_t size() const {
			return args.size();
		}

		/// @brief Get index from argument name.
		/// @param name The argument name.
		/// @param insert if true the argument will be inserted if not found.
		/// @return The argument index.
		size_t index(const char *name, bool insert = false, bool input = false);

		/// @brief Get index from argument name.
		/// @param name The argument name.
		/// @return The argument index.
		size_t index(const char *name) const;

		bool for_each(const std::function<bool(const size_t index, const char *name, const Value::Type type)> &call) const;
		bool for_each_input(const std::function<bool(const size_t index, const char *name, const Value::Type type)> &call) const;
		bool for_each_output(const std::function<bool(const size_t index, const char *name, const Value::Type type)> &call) const;

	};
	*/

 }
