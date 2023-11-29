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

			/// @brief Request method.
			const HTTP::Method method;

			/// @brief Request path.
			std::string path;

			/// @brief Index of arguments from path.
			size_t popptr = 0;

		protected:

		public:
			Request(const char *p, HTTP::Method m = HTTP::Get) : method{m}, path{p} {
			}

			Request(const char *p, const char *m) : method{HTTP::MethodFactory(m)}, path{p} {
			}

			/// @brief is request empty?
			inline bool empty() const noexcept {
				return path.empty();
			}

			/// @brief Execute request.
			/// @param response Object for request response.
			void exec(Response &response);

			/// @brief Execute request.
			/// @param response Object for request response.
			void exec(Report &response);

			inline operator const char *() const noexcept {
				return path.c_str();
			}

			inline operator HTTP::Method() const noexcept {
				return this->method;
			}

			inline bool operator==(HTTP::Method method) const noexcept {
				return this->method == method;
			}

			/// @brief Pop one element from path.
			virtual String pop();

			virtual Request & pop(std::string &value);
			virtual Request & pop(int &value);
			virtual Request & pop(unsigned int &value);

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
