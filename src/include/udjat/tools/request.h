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

#ifndef UDJAT_REQUEST_H_INCLUDED

	#define UDJAT_REQUEST_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <string>
	#include <cstring>
	#include <functional>
	#include <vector>
	#include <udjat/tools/timestamp.h>
	#include <udjat/tools/value.h>
	#include <memory>
	#include <udjat/tools/http/mimetype.h>
	#include <udjat/tools/method.h>
	#include <udjat/tools/string.h>
	#include <udjat/tools/http/mimetype.h>

	namespace Udjat {

		/// @brief Cache information for http responses.
		class UDJAT_API ResponseInfo {
		protected:
			/// @brief Expiration timestamp (For cache headers)
			time_t expiration = 0;

			/// @brief Timestamp of data.
			time_t modification = 0;

		public:
			constexpr ResponseInfo() {
			}

			/// @brief Set timestamp for cache the response.
			void setExpirationTimestamp(const time_t time);

			/// @brief Set timestamp for data.
			void setModificationTimestamp(const time_t time);

		};

		/// @brief Report in the format row/col.
		class UDJAT_API Report : public ResponseInfo {
		protected:

			struct {
				std::string caption;
			} info;

			struct {

				/// @brief Column names.
				std::vector<std::string> names;

				/// @brief Next column name.
				std::vector<std::string>::iterator current;

			} columns;

			/// @brief Start a new row.
			/// @return true if the row was open.
			virtual bool open();

			/// @brief End row.
			/// @return true if the row have columns.
			virtual bool close();

			/// @brief Get next column title (close and open row if needed).
			std::string next();

			Report();
			void set(const char *column_name, va_list args);

		public:

			/// @brief Set report title.
			/// @param caption	The report title.
			inline void caption(const char *value) noexcept {
				info.caption = value;
			}

			/// @brief Open report, define column names.
			/// @param name	Report	name.
			/// @param column_name	First column name.
			/// @param ...			Subsequent column names.
			void start(const char *column_name, ...) __attribute__ ((sentinel));

			/// @brief Open report, define column names.
			/// @param column_names	The column names.
			void start(const std::vector<std::string> &column_names);

			virtual ~Report();

			virtual Report & push_back(const char *str) = 0;

			virtual Report & push_back(const std::string &value);

			virtual Report & push_back(const short value);
			virtual Report & push_back(const unsigned short value);

			virtual Report & push_back(const int value);
			virtual Report & push_back(const unsigned int value);

			virtual Report & push_back(const long value);
			virtual Report & push_back(const unsigned long value);

			virtual Report & push_back(const TimeStamp value);
			virtual Report & push_back(const bool value);

			virtual Report & push_back(const float value);
			virtual Report & push_back(const double value);

			template <typename T>
			Report & push_back(const T &value) {
				return this->push_back(std::to_string(value));
			}

		};

		class UDJAT_API Response : public ResponseInfo, public Udjat::Value {
		public:
		protected:

			/// @brief Response type.
			MimeType type = MimeType::custom;

			/// @brief is the response valid?
			bool valid = true;

		public:

			constexpr Response(const MimeType m = MimeType::custom)
			: type(m) { }

			inline bool operator ==(const MimeType type) const noexcept {
				return this->type == type;
			}

			inline bool operator !=(const MimeType type) const noexcept {
				return this->type != type;
			}

			inline MimeType getType() const noexcept {
				return this->type;
			}

			inline operator MimeType() const noexcept {
				return this->type;
			}

		};

		class UDJAT_API Request {
		private:

			/// @brief Data from request path.
			struct PrivateData {
				/// @brief Request method.
				const HTTP::Method method;

				/// @brief Effective request path, after removal of versions, format type, etc.
				const char *popptr = "";

				constexpr PrivateData(const HTTP::Method m, const char *p = "") : method{m}, popptr{p} {
				}

			} request;

		protected:

			/// @brief The request API version.
			unsigned int apiver = 0;

			/// @brief The request mimetype.
			MimeType mimetype = MimeType::json;

			/// @brief Reset path processing, go to begin of path, reset api version and mime type from path (if available).
			Request & rewind();

			Request(HTTP::Method method = HTTP::Get);

			Request(const char *method);

		public:
			inline unsigned int version() const noexcept {
				return apiver;
			}

			/// @brief Get request property.
			/// @param name The property name
			/// @param def The default value.
			virtual String getProperty(const char *name, const char *def = "") const;

			/// @brief Get request property by index.
			/// @param index The property index
			/// @param def The default value.
			virtual String getProperty(size_t index, const char *def = "") const;

			inline String operator[](const char *name) const {
				return getProperty(name);
			}

			inline String operator[](size_t index) const {
				return getProperty(index);
			}

			/// @brief Execute request.
			/// @param response Object for request response.
			void exec(Response &response);

			/// @brief Execute request.
			/// @param response Object for request response.
			void exec(Report &response);

			/// @brief Get original request path.
			virtual const char *c_str() const noexcept;

			inline operator const char *() const noexcept {
				return c_str();
			}

			inline operator HTTP::Method() const noexcept {
				return request.method;
			}

			inline const char * path() const noexcept {
				return request.popptr;
			}

			inline bool operator==(HTTP::Method method) const noexcept {
				return this->request.method == method;
			}

			/// @brief Pop one element from path.
			String pop();

			Request & pop(std::string &value);
			Request & pop(int &value);
			Request & pop(unsigned int &value);

		};

	}

	namespace std {

		template <typename T>
		inline Udjat::Report & operator<<(Udjat::Report &out, T value) {
			return out.push_back(value);
		}

		template <typename T>
		inline Udjat::Request & operator>>(Udjat::Request &in, T &value) {
			return in.pop(value);
		}

	}


#endif // UDJAT_REQUEST_H_INCLUDED
