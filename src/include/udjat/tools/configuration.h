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
 #include <string>
 #include <mutex>
 #include <vector>
 #include <functional>
 #include <cstdint>
 #include <udjat/tools/string.h>

 namespace Udjat {

	namespace Config {

		UDJAT_API int32_t get(const std::string &group, const std::string &name, const int32_t def);
		UDJAT_API int64_t get(const std::string &group, const std::string &name, const int64_t def);
		UDJAT_API uint32_t get(const std::string &group, const std::string &name, const uint32_t def);
		UDJAT_API uint64_t get(const std::string &group, const std::string &name, const uint64_t def);
		UDJAT_API float get(const std::string &group, const std::string &name, const float def);
		UDJAT_API double get(const std::string &group, const std::string &name, const double def);
		UDJAT_API Udjat::String get(const std::string &group, const std::string &name, const char *def);
		UDJAT_API Udjat::String get(const std::string &group, const std::string &name, const std::string &def);
		UDJAT_API bool get(const std::string &group, const std::string &name, const bool def);

		/// @brief Enable loading of configuration from user's home dir.
		///	This method should be called BEFORE any other one, it doesnt work if file was already loaded.
		UDJAT_API void allow_user_homedir(bool allow = false) noexcept;

		/// @brief Navigate from all group keys.
		/// @param group Group name.
		/// @param call function to call on every group key until it returns 'true'.
		/// @return false if the lambda returns 'false' for all keys.
		UDJAT_API bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call);

		UDJAT_API bool hasGroup(const std::string &group);
		UDJAT_API bool hasKey(const char *group, const char *key);

		template <typename T>
		class UDJAT_API Value {
		private:
			T def;

			std::string group;
			std::string name;

		public:
			constexpr Value(const char *g, const char *n, const T d) : def(d),group(g),name(n) {
			}

			T get() const {
				return Config::get(group,name,def);
			}

			operator T() const {
				return get();
			}

			const std::string to_string() const {
				return Config::get(group,name,std::to_string(def));
			}

		};

		template <>
		class UDJAT_API Value<Udjat::String> : public Udjat::String {
		public:
			Value(const char *g, const char *n, const char *d = "")
				: Udjat::String{Config::get(g,n,d).c_str()} {
			}
		};

		template <>
		class UDJAT_API Value<std::string> : public std::string {
		public:
			Value(const char *g, const char *n, const char *d = "")
				: std::string{Config::get(g,n,d).c_str()} {
			}

			/// @brief Translate block ${name} in the string to *value.
			/// @param name Name of the block inside the string to substitute.
			/// @param value Value to substitute.
			Value<std::string> & set(const char *name, const char *value);

			/// @brief Translate block ${name} in the string with the value of group/key.
			/// @param name Name of the block inside the string to substitute.
			/// @param group Group of the configuration to get the value to substitute
			/// @param key Key with the value to substitute.
			/// @param def dafault value if the group/key doesn't exists in the configuration.
			Value<std::string> & set(const char *name, const char *group, const char *key, const char *def = "");

			inline operator const char *() const noexcept {
				return c_str();
			}

			inline const char *c_str() const noexcept {
				return std::string::c_str();
			}

			/// @brief Test if the string contains one of the elements of a list.
			/// @return Index of the matched content (negative if not found).
			/// @retval -ENODATA The string is empty.
			/// @retval -ENOENT The string dont match any of the values.
			int select(const char *value, va_list args) const noexcept;

			/// @brief Test if the string contains one of the elements of a list.
			/// @return Index of the matched content (negative if not found).
			/// @retval -ENODATA The string is empty.
			/// @retval -ENOENT The string dont match any of the values.
			int select(const char *value, ...) const noexcept __attribute__ ((sentinel));

		};

		template <>
		class UDJAT_API Value<std::vector<std::string>> : public std::vector<std::string> {
		public:
			Value(const char *group, const char *name, const char *def, const char *delim = ",");

		};


	}

 }

 /*
 namespace std {

	template <typename T>
	inline string to_string(const Udjat::Config::Value<T> &value) {
			return value.to_string();
	}

 }
 */
