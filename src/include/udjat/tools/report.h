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
 #include <udjat/defs.h>
 #include <udjat/tools/abstract/response.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/mimetype.h>
 #include <string>
 #include <vector>

 namespace Udjat {

	namespace Response {

		/// @brief Response in the format row/col.
		class UDJAT_API Table : public Abstract::Response {
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

			Table(const MimeType mimetype = MimeType::custom);
			void set(const char *column_name, va_list args);

			virtual void for_each(const std::function<void(const Value::Type type, const char *value)> &func) const = 0;

		public:

			/// @brief Set report title.
			/// @param caption	The report title.
			inline void caption(const char *value) noexcept {
				info.caption = value;
			}

			virtual bool empty() const = 0;

			/// @brief Open report, define column names.
			/// @param name	Report	name.
			/// @param column_name	First column name.
			/// @param ...			Subsequent column names.
			void start(const char *column_name, ...) __attribute__ ((sentinel));

			/// @brief Open report, define column names.
			/// @param column_names	The column names.
			void start(const std::vector<std::string> &column_names);

			void serialize(std::ostream &out) const;
			void serialize(std::ostream &out, const MimeType mimetype) const;

			std::string to_string() const;
			std::string to_string(const MimeType mimetype) const;

			virtual ~Table();

			virtual Table & push_back(const char *str, Udjat::Value::Type type = Udjat::Value::String) = 0;

			virtual Table & push_back(const std::string &value, Udjat::Value::Type type = Udjat::Value::String);

			virtual Table & push_back(const short value);
			virtual Table & push_back(const unsigned short value);

			virtual Table & push_back(const int value);
			virtual Table & push_back(const unsigned int value);

			virtual Table & push_back(const long value);
			virtual Table & push_back(const unsigned long value);

			virtual Table & push_back(const TimeStamp value);
			virtual Table & push_back(const bool value);

			virtual Table & push_back(const float value);
			virtual Table & push_back(const double value);

			template <typename T>
			Table & push_back(const T &value) {
				return this->push_back(std::to_string(value));
			}

		};

	}

 }

 namespace std {

	template <typename T>
	inline Udjat::Response::Table & operator<<(Udjat::Response::Table &out, T value) {
		return out.push_back(value);
	}

	inline string to_string(const Udjat::Response::Table &response) noexcept {
		return response.to_string();
	}

	inline ostream & operator<< (ostream& os, const Udjat::Response::Table &response) {
		return os << response.to_string();
	}

 }


