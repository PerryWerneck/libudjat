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
 #include <udjat/authentication.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/variant.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/method.h>

 namespace Udjat {

	/// @brief Base API request.
	class UDJAT_API Request : public Udjat::Variant {
	private:

		/// @brief Current argument.
		const char *argptr = "";

		/// @brief The processed request path.
		const char *reqpath = "";

	protected:

		/// @brief Set request path.
		/// @param path The new request path.
		inline void reset(const char *path = "") noexcept {
			argptr = reqpath = path;
		}

		/// @brief The requested API version, 0 if this is not an API call.
		unsigned int apiver = 0;

		/// @brief Authentication for this request.
		std::shared_ptr<Authentication> auth;

		/// @brief Extract prefix /api/[APIVERSION] from string.
		/// @param path The request path beginning with /api/ (Will be updated).
		/// @param apiver The extracted API version (will be 0 if the request doesnt begin with /api/)
		/// @return true if the request begin with /api/ 
		static bool pop(const char * &path, unsigned int &apiver);

	public:

		Request(const char *path = "");

		virtual ~Request();

		inline unsigned int version() const noexcept {
			return apiver;
		}

		inline bool apicall() const noexcept {
			return apiver != 0;
		}

		/// @bref Check if request is for root page.
		/// @return true if the path is "/"
		inline bool root() const noexcept {
			return reqpath[0] == '/' && reqpath[1] == 0;
		}

		virtual MimeType mimetype() const noexcept;

		virtual HTTP::Method method() const noexcept;

		/// @brief Check and extract element from path.
		/// @param prefix The prefix to check and extract.
		/// @param path The current path.
		/// @return true if the prefix was found and extracted.
		static bool pop(const char *prefix, const char * &path) noexcept;

		/// @brief Extract first element from path.
		/// @param out String to receive the extracted element.
		/// @param path The current path.
		/// @return true if first element was found and extracted.
		static bool pop(std::string &out, const char * &path) noexcept;

		/// @brief Test and extract request path.
		/// @param key The key to check.
		/// @return true if the request path was equal and it was removed, request is now at next element.
		inline bool pop(const char *key) noexcept {
			return pop(key,argptr);
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

		/// @brief Get the user authentication level.
		/// @return The authentication level for current user.
		/// @retval Authentication::None if not authenticated.
		Authentication::Role role() const noexcept;

		/// @brief Test if the request can handle the path.
		/// @param prefix The path being searched.
		/// @return true if the request path starts with prefix.
		bool operator ==(const char *prefix) const noexcept;

#if __cplusplus >= 202002L

		int operator <=>(const Authentication::Role role) const noexcept {
			return this->role() - role;
		}

#else

		inline bool operator ==(const Authentication::Role role) const noexcept {
			return this->role() == role;
		}

		inline bool operator>(const Authentication::Role role) const noexcept {
			return this->role() > role;
		}

		inline bool operator<(const Authentication::Role role) const noexcept {
			return this->role() < role;
		}

		inline bool operator>=(const Authentication::Role role) const noexcept {
			return this->role() >= role;
		}

		inline bool operator<=(const Authentication::Role role) const noexcept {
			return this->role() <= role;
		}

#endif

		/// @brief Check the required role for this request.
		/// @param role The current user role.
		/// @return true if the user has access to this request.
		inline bool allow(const Authentication::Role role = Authentication::None) const {
			return authentication()->allow(role);
		}

		/// @brief Get authentication token.
		inline std::shared_ptr<Authentication> authentication() const noexcept {
			return auth;
		};

		/// @brief Get the username for the request.
		/// @return The username if authenticated, empty string if not.
		const char *username() const;

		// bool get_property(const char *key, std::string &value) const override;
		bool get_property(const char *key, Udjat::Variant &value) const override;

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
		inline const char *c_str() const noexcept {
			return reqpath;
		}

		inline operator const char *() const noexcept {
			return c_str();
		}

		/// @brief Get current request path (after 'pop()').
		/// @see pop()
		/// @return The path remaining after 'pop()' calls.
		const char * path() const noexcept;

		/// @brief Scan path prefix from a list of options, if found remove the option from path..
		/// @return Index of the action fround and removed from path;.
		/// @retval -EINVAL The current path doesnt start with '/'
		/// @retval -ENODATA The request is empty.
		/// @retval -ENOENT The action is not in the list.
		/// @see pop()
		int select(const char *value, ...) noexcept __attribute__ ((sentinel));

		/// @brief Pop one element from path.
		/// @return The first element from current path.
		/// @see path()
		Udjat::String pop();

		Request & pop(std::string &value);
		Request & pop(int &value);
		Request & pop(unsigned int &value);

		/// @brief Write message to log.
		/// Write text message to system log adding request info.
		virtual void logger(Logger::Level level, const char *domain, const char *text) const noexcept;

		inline void info(const char *domain, const char *text) const noexcept {
			logger(Logger::Info,domain,text);
		}

		inline void warning(const char *domain, const char *text) const noexcept {
			logger(Logger::Warning,domain,text);
		}

		inline void error(const char *domain, const char *text) const noexcept {
			logger(Logger::Error,domain,text);
		}

		inline void notice(const char *domain, const char *text) const noexcept {
			logger(Logger::Notice,domain,text);
		}

#ifdef LOG_DOMAIN
		inline void info(const char *text) const noexcept {
			logger(Logger::Info,LOG_DOMAIN,text);
		}

		inline void warning(const char *text) const noexcept {
			logger(Logger::Warning,LOG_DOMAIN,text);
		}

		inline void error(const char *text) const noexcept {
			logger(Logger::Error,LOG_DOMAIN,text);
		}

		inline void notice(const char *text) const noexcept {
			logger(Logger::Notice,LOG_DOMAIN,text);
		}
#endif

	};

 }

 namespace std {

	template <typename T>
	inline Udjat::Request & operator>>(Udjat::Request &in, T &value) {
		return in.pop(value);
	}

 }

