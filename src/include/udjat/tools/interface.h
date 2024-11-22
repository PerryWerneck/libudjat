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
  * @brief Declares the abstract interface for API calls.
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

	/// @brief Abstract interface for API calls.
	class UDJAT_API Interface {
	private:

		/// @brief The method name.
		const char *_name;
		class Controller;

	protected:

		typedef Interface Super;

		/// @brief Copy input values from object to value.
		/// @param in The object to get the input properties
		/// @param out The 'worker' object, will receive the input properties.
		void get_inputs(const Abstract::Object &in, Udjat::Value &out) const;

		/// @brief Build an interface.
		/// @param name The interface name in (A single word).
		Interface(const char *name);

		/// @brief Build an interface from XML description
		/// @param name The interface declaration.
		Interface(const XML::Node &node);

	public:

		virtual ~Interface();

		static Interface & find(const char *name);

		inline const char * name() const noexcept {
			return _name;
		}

		/// @brief Get full name of interface (example: br.eti.werneck.udjat.agent)
		/// @return The full name of this interface.
		std::string to_string() const;

#if __cplusplus >= 202002L
		inline auto operator <=>(const char *name) const noexcept {
			return strcasecmp(name,this->_name);
		}
#else
		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,this->_name) == 0;
		}
#endif

		/// @brief Enum interfaces.
		/// @param call The callback, returns true to interrupt the loop.
		/// @return true if the loop was interrupted
		static bool for_each(const std::function<bool(const Interface &intf)> &call);

		/// @brief Enum interface properties.
		/// @param call The callback to handle property, returns true to interrupt the loop.
		/// @return true if the loop was interrupted
		virtual bool for_each(const std::function<bool(const size_t index, bool input, const char *name, const Value::Type type)> &call) const;

		/// @brief Execute chained action (for scripts using multiple interfaces).
		/// @param path The path for object request.
		/// @param values The in/out values.
		virtual void call(const char *path, Udjat::Value &values) = 0;

		/// @brief Execute request, return response.
		/// @param request The client request.
		/// @param response The response.
		virtual void call(Request &request, Response &response);

		/// @brief Execute chained action (for scripts using multiple interfaces).
		/// @param name The interface name.
		/// @param path The path for object request.
		/// @param values The in/out values.
		static void call(const char *name, const char *path, Udjat::Value &values);

		/// @brief Execute request, return response.
		/// @param name The interface name.
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
