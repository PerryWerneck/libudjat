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
 #include <iconv.h>
 #include <mutex>

 namespace Udjat {

	namespace Win32 {

		class UDJAT_API Charset {
		private:

			static std::mutex guard;
			iconv_t icnv;

		public:

			Charset(const char *from, const char *to);
			~Charset();

			/// @brief Get win32 local charset.
			const char * system();

			void convert(const char *from, std::string &to);
			std::string convert(const char *text);

			static std::string from_windows(const char *winstr);
			static std::string to_windows(const char *utfstr);

		};

		/// @brief Convert Win32 string to UTF8String.
		class UDJAT_API UTF8String : public std::string, private Charset {
		public:
			UTF8String();
			UTF8String(const char *winstr);
			~UTF8String();

			/// @brief Assign Win32 string, will be converted to UTF-8.
			UTF8String & assign(const char *winstr);

		};

		/// @brief Convert UTF8 string to Win32 String.
		class UDJAT_API Win32String : public std::string, private Charset {
		public:
			Win32String();
			Win32String(const char *utfstr);
			~Win32String();

			/// @brief Assign UTF-8 string, will be converted to Win32.
			Win32String & assign(const char *utfstr);

		};

	}

 }
