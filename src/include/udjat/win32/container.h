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
 #include <stdexcept>
 #include <udjat/win32/exception.h>

 namespace Udjat {

	namespace Win32 {

		template <typename T>
		class Container {
		private:
			T * contents = nullptr;

		protected:
			virtual DWORD load(T *buffer, ULONG *ifbuffersize) = 0;

			void load() {

				ULONG ifbuffersize = sizeof(T)*2;
				contents = (T *) malloc(ifbuffersize+1);
				DWORD rc = -1;

				for(size_t ix = 0; ix < 4; ix++) {

					memset(contents,0,ifbuffersize+1);

					rc = load(contents,&ifbuffersize);
					switch(rc) {
					case NO_ERROR:
						return;

					case ERROR_INSUFFICIENT_BUFFER:
					case ERROR_BUFFER_OVERFLOW:
						contents = (T *) realloc(contents,ifbuffersize+1);
						break;

					default:
						throw Win32::Exception(rc);

					}

				}

				throw Win32::Exception(rc);

			}

		public:

			constexpr Container() {
			}

			inline const T * get() {
				if(!contents) {
					load();
				}
				return contents;
			}

			~Container() {
				if(contents) {
					free(contents);
				}
			}

		};

	}

 }

