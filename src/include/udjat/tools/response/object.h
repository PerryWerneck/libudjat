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
 #include <udjat/tools/response/value.h>
 #include <udjat/tools/value.h>
 #include <map>

 namespace Udjat {

 	namespace Response {

		/// @brief Object response.
		class UDJAT_API Object : public Value {
		private:

			class Value;
			Udjat::Value::Type type = Udjat::Value::Object;
			std::map<std::string,Value> children;

		public:
			Object(const MimeType mimetype = MimeType::json);
			virtual ~Object();

			operator Type() const noexcept override;

			bool empty() const noexcept override;

			bool for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const override;
			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Udjat::Value::Type type = Udjat::Value::Object) override;
			Udjat::Value & reset(const Udjat::Value::Type type) override;
			Udjat::Value & set(const char *value, const Type type = String) override;

		};

 	}

 }

