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

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/tools/value.h>

 namespace Udjat {

	/*
	class UDJAT_PRIVATE Tester : public Udjat::Value {
	private:
		Value::Type type = Udjat::Value::Undefined;

		union Value {
			float f;
			bool b;
			unsigned long u;
			long s;
			time_t t;
			double r;
		} value;

	public:
		Tester() {
		};

		virtual ~Tester() {
		}

		bool operator==(const Tester &src) const {

			if(src.type != this->type) {
				return false;
			}

			switch(this->type) {
			case Udjat::Value::Undefined:
				return true;

			case Udjat::Value::Object:
			case Udjat::Value::Array:
			throw system_error(ENOTSUP,system_category(),"Invalid operation for this value");

			case Udjat::Value::Real:
			case Udjat::Value::Fraction:
				return this->value.r == src.value.r;

			case Udjat::Value::Signed:
				return this->value.s == src.value.s;

			case Udjat::Value::Unsigned:
				return this->value.u == src.value.u;

			case Udjat::Value::Timestamp:
				return this->value.t == src.value.t;

			case Udjat::Value::Boolean:
				return this->value.b == src.value.b;

			case Udjat::Value::String:
				throw system_error(ENOTSUP,system_category(),"Unable to test strings");

			}

			throw system_error(ENOTSUP,system_category(),"Invalid operation for this value");
		}

		bool isNull() const override {
			return type == Udjat::Value::Undefined;
		}

		/// @brief Clear contents, set value type.
		Udjat::Value & reset(const Type type) override {
			this->type = type;
			return *this;
		}

		Udjat::Value & setFraction(const float fraction) override {
			type = Udjat::Value::Fraction;
			value.r = fraction;
			return *this;
		}

		Udjat::Value & set(const short value) {
			type = Udjat::Value::Signed;
			this->value.s = value;
			return *this;
		}

		Udjat::Value & set(const unsigned short value) {
			type = Udjat::Value::Unsigned;
			this->value.u = value;
			return *this;
		}

		Udjat::Value & set(const int value) {
			type = Udjat::Value::Signed;
			this->value.s = value;
			return *this;
		}

		Udjat::Value & set(const unsigned int value) {
			type = Udjat::Value::Unsigned;
			this->value.u = value;
			return *this;
		}

		Udjat::Value & set(const long value) {
			type = Udjat::Value::Signed;
			this->value.s = value;
			return *this;
		}

		Udjat::Value & set(const unsigned long value) {
			type = Udjat::Value::Unsigned;
			this->value.u = value;
			return *this;
		}

		Udjat::Value & set(const TimeStamp value) {
			type = Udjat::Value::Timestamp;
			this->value.t = (time_t) value;
			return *this;
		}

		Udjat::Value & set(const bool value) {
			type = Udjat::Value::Boolean;
			this->value.b = value;
			return *this;
		}

		Udjat::Value & set(const float value) {
			type = Udjat::Value::Real;
			this->value.r = value;
			return *this;
		}

		Udjat::Value & set(const double value) {
			type = Udjat::Value::Real;
			this->value.r = value;
			return *this;
		}

	};
	*/

 }
