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
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/string.h>

 namespace Udjat {

	/// @brief Base API request.
	class UDJAT_API Request : public Udjat::Value {
	private:

		/// @brief Current argument.
		const char *argptr = "";

		/// @brief The processed request path.
		const char *reqpath = "";

		/// @brief Looks whether the request path begins with prefix.
		/// @param prefix to check.
		/// @return true if the request path begins with the argument.
		bool has_prefix(const char *prefix) const noexcept;

	protected:

		/// @brief Set request path.
		/// @param path The new request path.
		inline void reset(const char *path = "") noexcept {
			argptr = reqpath = path;
		}

		/// @brief The requested API version.
		unsigned int apiver = 0;

	public:

#if __cplusplus >= 201703L
		constexpr Request(const char *path = "") : argptr{path}, reqpath{path} {
		}
#else
		Request(const char *path = "") : argptr{path}, reqpath{path} {
		}
#endif

		virtual ~Request();

		inline unsigned int version() const noexcept {
			return apiver;
		}

		/// @brief Get request header.
		/// @param name Name of the header.
		/// @return The header value of "" if not found.
		virtual const char * header(const char *name) const noexcept;

		/// @brief Enumerate all request properties.
		/// @param call Method to call in every request property.
		/// @return True if the enumeration was interrupt with return 'true' from call();
		virtual bool for_each(const std::function<bool(const char *name, const char *value)> &call) const;

		/// @brief Is the request empty?
		/// @return True if the request path is empty.
		inline bool empty() const noexcept {
			return !(reqpath && *reqpath);
		}

		bool getProperty(const char *key, std::string &value) const override;

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
		/// @param def The value to return if the string dont have query.
		/// @return The query value or 'def'.
		virtual const char * query(const char *def = "") const;

		/// @brief Reset argument parser, next pop() will return the first element from path.
		/// @see pop
		inline Request & rewind() noexcept {
			argptr = reqpath;
			return *this;
		}

		/// @brief Get original request path.
		const char *c_str() const noexcept;

		inline operator const char *() const noexcept {
			return c_str();
		}

		/// @brief Get current request path (after 'pop()').
		/// @see pop()
		/// @return The path remaining after 'pop()' calls.
		const char * path() const noexcept;

		/// @brief Test if the request can handle the path.
		/// @param prefix The path being searched.
		/// @return true if the request path starts with prefix.
		inline bool operator==(const char *prefix) const noexcept {
			return has_prefix(prefix);
		}

		/// @brief pop() first element from path, select it from list.
		/// @return Index of the selected action or negative if not found.
		/// @retval -ENODATA The request is empty.
		/// @retval -ENOENT The action is not in the list.
		/// @see pop()
		int select(const char *value, ...) noexcept __attribute__ ((sentinel));

		/// @brief Test and extract request path.
		/// @param path The path to check.
		/// @return true if the request path was equal and it was removed, request is now at next parameter.
		bool pop(const char *path) noexcept;

		/// @brief Pop one element from path.
		/// @return The first element from current path.
		/// @see path()
		Udjat::String pop();

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

