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
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response/value.h>
 #include <udjat/tools/response/table.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/string.h>

 namespace Udjat {

	class UDJAT_API Request : public Abstract::Object {
	private:

		/// @brief Request method.
		const HTTP::Method method;

		/// @brief Current argument.
		const char *argptr = nullptr;

		const char * chk_prefix(const char *arg) const noexcept;

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

		UDJAT_DEPRECATED(const char * getPath() const noexcept);

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

		/// @brief Navigate from all arguments and properties until 'call' returns true.
		/// @return true if 'call' has returned true, false if not.
		virtual bool for_each(const std::function<bool(const char *name, const Value &value)> &call) const;

		/// @brief Navigate from all arguments and properties until 'call' returns true.
		/// @return true if 'call' has returned true, false if not.
		virtual bool for_each(const std::function<bool(const char *name, const char *value)> &call) const;

		/// @brief Get property/argument value.
		/// @param key The property name or index.
		/// @param value String to update with the property value.
		/// @return true if the property is valid.
		bool getProperty(const char *key, std::string &value) const override;
		virtual bool getProperty(size_t ix, std::string &value) const;

		/// @brief Get property value.
		/// @param key The property name.
		/// @param value Object to receive the value.
		/// @return true if the property is valid and value was updated.
		bool getProperty(const char *key, Udjat::Value &value) const override;

		/// @brief Get property value.
		/// @param key The property name.
		/// @return String with property value or empty if not found.
		virtual String operator[](const char *key) const;

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

		inline HTTP::Method verb() const noexcept {
			return this->method;
		}

		UDJAT_DEPRECATED(inline HTTP::Method as_type() const noexcept) {
			return this->method;
		}

		/// @brief Get current request path (after 'pop()').
		/// @see pop()
		/// @return The path remaining after 'pop()' calls.
		const char * path() const noexcept;

		inline bool operator==(HTTP::Method method) const noexcept {
			return this->method == method;
		}

		/// @brief Test if the request can handle the path.
		/// @param prefix The path being searched.
		/// @return true if the request path starts with prefix.
		inline bool operator==(const char *prefix) const noexcept {
			return chk_prefix(prefix) != nullptr;
		}

		/// @brief pop() first element from path select it from list.
		/// @return Index of the selected action or negative if not found.
		/// @retval -ENODATA The request is empty.
		/// @retval -ENOENT The action is not in the list.
		/// @see pop()
		int select(const char *value, ...) noexcept __attribute__ ((sentinel));

		/// @brief Test and extract request path.
		/// @param path The path to check.
		/// @return true if the request path was equal and it was removed, request is now at first parameter.
		bool pop(const char *path) noexcept;

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

