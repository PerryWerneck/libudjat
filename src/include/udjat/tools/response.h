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
 #include <udjat/tools/abstract/response.h>
 #include <udjat/tools/value.h>
 #include <stdexcept>

 namespace Udjat {

 	namespace Response {

		class UDJAT_API Value : public Abstract::Response, public Udjat::Value {
		public:
			Value(const MimeType mimetype = MimeType::custom) : Abstract::Response{mimetype} {
			}

			operator Value::Type() const noexcept override;

			bool isNull() const override;
			Udjat::Value & reset(const Udjat::Value::Type type) override;
			Udjat::Value & set(const Udjat::Value &value) override;

			void serialize(std::ostream &out) const;
			void serialize(std::ostream &out, const MimeType mimetype) const override;

			std::string to_string() const;

		};

 	}

 }

 namespace std {

	inline string to_string(const Udjat::Response::Value &response) noexcept {
		return response.to_string();
	}

	inline ostream & operator<< (ostream& os, const Udjat::Response::Value &response) {
		return os << response.to_string();
	}

 }
