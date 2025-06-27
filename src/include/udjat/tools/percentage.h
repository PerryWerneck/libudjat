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

 #pragma once

 #include <udjat/defs.h>
 #include <sstream>
 #include <iomanip>

 #if __GNUC__ > 10
 namespace Udjat {

	typedef float Percentage;

 }

 namespace std {

	inline string to_string(const Udjat::Percentage percentage) {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << (((float) (percentage)) * 100.0) << "%";
		return stream.str();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Percentage percentage ) {
		os << std::fixed << std::setprecision(2) << (((float) (percentage)) * 100.0) << "%";
		return os;
	}

 }
 #else

 namespace Udjat {

	struct UDJAT_API Percentage {
		float value = 0.0;

		constexpr Percentage(int v) : value{(float) v} {
		}

		constexpr Percentage(float v) : value{v} {
		}

		constexpr Percentage(double v) : value{(float) v} {
		}
	
		template <typename T>
		inline Percentage& operator=(const T v) {
			value = (float) v;
			return *this;
		}

		inline operator float() const noexcept {
			return value;
		}

	};


	
 }

 namespace std {

	inline string to_string(const Udjat::Percentage &percentage) {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << (((float) (percentage.value)) * 100.0) << "%";
		return stream.str();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Percentage &percentage ) {
		os << std::fixed << std::setprecision(2) << (((float) (percentage.value)) * 100.0) << "%";
		return os;
	}

  }
 
 #endif // __GCC__ > 10


