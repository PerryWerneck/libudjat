/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/method.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/report.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/string.h>

 namespace Udjat {

	class UDJAT_API Request {
	private:

		/// @brief Request method.
		const HTTP::Method method;

		/// @brief Current argument.
		const char *argptr = nullptr;

	protected:

		/// @brief The processed request path.
		const char *reqpath = "";

		/// @brief The requested API version.
		unsigned int apiver = 0;

	public:

		constexpr Request(const char *path, HTTP::Method m = HTTP::Get) : method{m}, reqpath{path} {
		}

		Request(const char *path, const char *method) : Request{path,HTTP::MethodFactory(method)} {
		}

		inline unsigned int version() const noexcept {
			return apiver;
		}

		/// @brief Is the request empty?
		/// @return True if the request path is empty.
		inline bool empty() const noexcept {
			return !(reqpath && *reqpath);
		}

		/// @brief Is this request authenticated?
		/// @return True if the request has user credentials.
		virtual bool authenticated() const noexcept;

		/// @brief Check the cache state.
		/// @param timestamp Current response timestamp.
		/// @return True if the cache can be used.
		/// @retval true The cache contents are valid.
		/// @retval false The cache must be refreshed.
		virtual bool cached(const TimeStamp &timestamp) const;

		/// @brief Get query.
		/// @param def The value to return if the string don have query.
		/// @return The query value or 'def'.
		virtual const char * query(const char *def = "") const;

		/// @brief Get argument.
		/// @param name The argument name
		/// @param def The default value.
		virtual String getArgument(const char *name, const char *def = "") const;

		/// @brief Get request property.
		/// @param name The property name
		/// @param def The default value.
		virtual String getProperty(const char *name, const char *def = "") const;

		/// @brief Get request property by index.
		/// @param index The property index
		/// @param def The default value.
		virtual String getProperty(int index, const char *def = "") const;

		/// @brief Get argument ou property, first search for argument if not found search for property.
		/// @param name The argument or property name.
		/// @return The argument or property, empty if not found.
		String operator[](const char *name) const;

		inline String operator[](size_t index) const {
			return getProperty(index);
		}

		/// @brief Reset argument parser, next pop() will return the first element from path.
		/// @see pop
		inline Request & rewind() noexcept {
			argptr = reqpath;
			return *this;
		}

		/// @brief Get original request path.
		virtual const char *c_str() const noexcept;

		inline operator const char *() const noexcept {
			return c_str();
		}

		inline operator HTTP::Method() const noexcept {
			return this->method;
		}

		/// @brief Get current request path (after 'pop()').
		/// @see pop()
		/// @return The path remaining after 'pop()' calls.
		const char * path() const noexcept;

		inline bool operator==(HTTP::Method method) const noexcept {
			return this->method == method;
		}

		/// @brief Test if current path is equal path();
		bool operator==(const char *path) const noexcept;

		/// @brief pop() first element from path select it from list.
		/// @return Index of the selected action or negative if not found.
		/// @retval -ENODATA The request is empty.
		/// @retval -ENOENT The action is not in the list.
		/// @see pop()
		int select(const char *value, ...) noexcept __attribute__ ((sentinel));

		/// @brief Pop one element from path.
		/// @return The first element from current path.
		/// @see path()
		String pop();

		Request & pop(std::string &value);
		Request & pop(int &value);
		Request & pop(unsigned int &value);

	};

 }

 namespace std {

	template <typename T>
	inline Udjat::Request & operator>>(Udjat::Request &in, T &value) {
		return in.pop(value);
	}

 }

