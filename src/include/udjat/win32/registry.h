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
 #include <functional>

 #ifndef _WIN32
	#error Registry objects requires win32
 #endif // _WIN32

 namespace Udjat {

	namespace Win32 {

		class UDJAT_API Registry {
		protected:
			HKEY hKey = 0;

			static std::string get(HKEY hK, const char *name, const char *def);
			static void set(HKEY hK, const char *name, const char *value);

		public:
			constexpr Registry(HKEY k) : hKey(k) {
			}

			Registry(bool write = false);
			Registry(const char *path, bool write = false);

			bool hasKey(const char *name) const noexcept;
			bool hasValue(const char *name) const noexcept;

			inline operator bool() const noexcept {
				return hKey != 0;
			}

			static HKEY open(HKEY hParent = HKEY_LOCAL_MACHINE, const char *path = nullptr, bool write = false);

			~Registry();

			/// @brief Remove key.
			/// @param Name Key name.
			void remove(const char *keyname);

			std::string get(const char *name, const char *def) const;
			DWORD get(const char *name, DWORD def) const;

			void set(const char *name, const char *value);

			inline void set(const char *name, const std::string &value) {
				set(name,value.c_str());
			}

			template<typename T>
			inline void set(const T &value) {
				return set(std::to_string(value).c_str());
			}

			bool for_each(const char *group, const std::function<bool(const char *key, const char *value)> &call);

		};

	}

 }


