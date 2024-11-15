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
  * @brief Declares an API call.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response/value.h>
 #include <vector>

 namespace Udjat {

	/// @brief Convenience class for XML defined methods.
	class UDJAT_API Method {
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

	protected:

		typedef Method Super;
		Method(const XML::Node &node);

	public:
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

 }
