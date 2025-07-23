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
 #include <udjat/tools/string.h>

 #ifndef _WIN32
	#error Registry objects requires win32
 #endif // _WIN32

 namespace Udjat {

	namespace Win32 {

		class UDJAT_API Registry {
		protected:
			HKEY hKey = 0;

			static Udjat::String get(HKEY hK, const char *name, const char *def);

			/// @brief Get binary data from registry.
			/// @param hK the registry key.
			/// @param name The value name.
			/// @param ptr Pointer to data.
			/// @param len Length of data.
			/// @return ptr on success, nullptr on failure.
			static void * get(HKEY hK, const char *name, void *ptr, size_t len);

			static void set(HKEY hK, const char *name, const char *value);
			static void set(HKEY hK, const char *name, const void *ptr, size_t len);

		public:
			constexpr Registry(HKEY k) : hKey(k) {
			}

			/// @brief Set registry application root.
			/// @param path The new registry path for application (should be a quark or static string)
			static void setRoot(const char *path = "SOFTWARE");

			Registry(HKEY k, bool write = false);

			Registry(HKEY k, const char *path, bool write = false);

			/// @brief  Open default registry.
			/// @param write true if not read-only.
			Registry(bool write = false);

			/// @brief  Open registry on path.
			/// @param path Registry path, start it with '\' to full registry path instead of application one.
			/// @param write true if not read-only.
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

			Udjat::String get(const char *name, const char *def) const;
			DWORD get(const char *name, DWORD def) const;
			UINT64 get(const char *name, UINT64 def) const;

			/// @brief Get binary data from registry.
			/// @param name The value name.
			/// @param ptr Pointer to data.
			/// @param len Length of data.
			/// @return ptr on success, nullptr on failure.
			void * get(const char *name, void *ptr, size_t len);

			void set(const char *name, const char *value);
			void set(const char *name, const void *ptr, size_t length);

			inline void set(const char *name, const std::string &value) {
				set(name,value.c_str());
			}

			template<typename T>
			inline void set(const T &value) {
				return set(std::to_string(value).c_str());
			}

			/// @brief Navigate from all group keys.
			/// @param group Group name.
			/// @param call function to call on every group key until it returns 'true'.
			/// @return false if call() returns 'false' for all keys.
			bool for_each(const char *group, const std::function<bool(const char *key, const char *value)> &call);

		};

	}

 }


