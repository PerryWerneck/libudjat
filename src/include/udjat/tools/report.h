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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/value.h>
 #include <string>
 #include <ostream>
 #include <vector>
 #include <list>
 #include <cstdarg>

 namespace Udjat {

	/// @brief Report with fixed columns.
	class UDJAT_API Report {
	private:

		struct Cell {
			Value::Type type = Value::Undefined;
			union {
				time_t timestamp;
				int sig;
				unsigned int unsig;
				double dbl;
				void *ptr = nullptr;
			} data;

			Cell();
			~Cell();

			void clear() noexcept;

			void set(const char *value);
			inline void set(const std::string &value) {
				return set(value.c_str());
			}

			void set(const short value);
			void set(const unsigned short value);
			void set(const int value);
			void set(const unsigned int value);
			void set(const TimeStamp &value);
			void set(const bool value);
			void set(const float value);
			void set(const double value);

			template <typename T>
			void set(const T &value) {
				return this->set(std::to_string(value));
			}

			void serialize(std::ostream &out) const;

		};

		struct {
			std::string caption;
		} field;

		std::list<Cell> cells;
		std::vector<std::string> headers;

		void set_headers(const char *column_name, va_list args);

	public:

		Report(const char *column_name, va_list args) {
			set_headers(column_name, args);
		}

		Report(const char *column_name, ...) __attribute__ ((sentinel));
		~Report();

		inline void caption(const char *str) noexcept {
			field.caption = str;
		}

		inline const char *caption() const noexcept {
			return field.caption.c_str();
		}

		template <typename T>
		void push_back(const T &value) {
			cells.emplace_back().set(value);
		}

		void to_json(std::ostream &out) const;
		void to_xml(std::ostream &out) const;
		void to_html(std::ostream &out) const;
		void to_yaml(std::ostream &out, size_t left_margin = 0) const;
		void to_sh(std::ostream &stream) const;
		void to_csv(std::ostream &out, char delimiter = ',') const;

	};


 }

