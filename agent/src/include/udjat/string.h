/*
 *
 * Copyright (C) <2019> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 *
 * @file
 *
 * @brief
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 *
 */

#ifndef UDJAT_STRING_H_INCLUDED

	#define UDJAT_STRING_H_INCLUDED 1

	#include <udjat/defs.h>
	#include <string>
	#include <ctime>
	#include <iostream>

	#ifndef DEFAULT_TIMESTAMP_FORMAT
		#define DEFAULT_TIMESTAMP_FORMAT "%d/%m/%Y %H:%M:%S"
	#endif

	namespace Udjat {

		class DLL_PUBLIC String : public std::string {
		public:
			String() = default;
			String(const char *ptr) : std::string(ptr) { };
			String(const char *ptr, size_t sz) : std::string(ptr,sz) { };

			template<typename T>
			inline String(const T val) : std::string() {
				this->assign(val);
			}

			template<typename T, typename... Targs>
			String(T value, Targs... Fargs) : String(value) {
				this->append(Fargs...);
			}

			String & set(const char *fmt, ...) AS_PRINTF(2,3);
			String & set_va(const char *fmt, va_list va);

			String & setTimeStamp(std::time_t tm = time(nullptr), const char *format = DEFAULT_TIMESTAMP_FORMAT) noexcept;
			String & setTimeStamp(const std::tm &tm, const char *format = DEFAULT_TIMESTAMP_FORMAT) noexcept;

			String & capitalize() noexcept;

			inline String & append() noexcept {
				return *this;
			}

			inline String & append(const char * value) {
				std::string::append(value);
				return *this;
			}

			inline String & append(char * value) {
				std::string::append(value);
				return *this;
			}

			inline String & append(const std::string & value) {
				std::string::append(value);
				return *this;
			}

			template<typename T>
			inline String & append(const T value) {
				std::string::append(std::to_string(value));
				return *this;
			}

			inline String & assign(const char * value) {
				std::string::assign(value);
				return *this;
			}

			inline String & assign(char * value) {
				std::string::assign(value);
				return *this;
			}

			template<typename T>
			String & assign(T value) {
				std::string::assign(std::to_string(value));
				return *this;
			}

			template<typename T, typename... Targs>
			String & append(T value, Targs... Fargs) {
				this->append(value);
				return this->append(Fargs...);
			}

			static bool hasSuffix(const char *str, const char *suffix) noexcept;
			static bool hasPrefix(const char *str, const char *prefix) noexcept;

		};

		template <typename T>
		inline String & operator<<(String & out, const T value) {
			return out.append(value);
		}

	}

	/*
	namespace std {

		inline std::string to_string( const Udjat::TimeStamp &value ) {
			return value.toString();
		}

	}
	*/


	#ifdef DEBUG

		#define debug(...) std::clog << Udjat::String(__FILE__, "(", __LINE__, ") ", __VA_ARGS__) << std::endl;

	#else

		#define debug(...) /* __VA_ARGS__ */

	#endif // DEBUG

	template<typename... Targs>
	void info(Targs... args) noexcept {
		std::cout << "core" << "\t" << Udjat::String(args...) << std::endl;
	}

	template<typename... Targs>
	void warning(Targs... args) noexcept {
		std::clog << "core" << "\t" << Udjat::String(args...) << std::endl;
	}

	template<typename... Targs>
	void error(Targs... args) noexcept {
		std::cerr << "core" << "\t" << Udjat::String(args...) << std::endl;
	}


#endif // UDJAT_STRING_H_INCLUDED

