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
 #include <netcon.h>
 #include <udjat/win32/exception.h>
 #include <comdef.h>

 namespace Udjat {

 	namespace Win32 {

		namespace Com {

			void UDJAT_API initialize();

			/// @brief Template for Object.
			template <typename O>
			class UDJAT_API Object {
			protected:
				O *object = nullptr;

			public:
				constexpr Object() {
				}

				constexpr Object(O *o) : object{o} {
				}

				virtual ~Object() {
					if(object) {
						object->Release();
					}
				}

				inline O * operator->() noexcept {
					return object;
				}

				inline O & operator *() noexcept {
					return *object;
				}

				inline O & get() noexcept {
					return *object;
				}

				inline void ** ptr() noexcept {
					return (void **) &object;
				}

			};

			/// @brief Template for Instance
			/// @tparam O The win32 object.
			template <typename O>
			class UDJAT_API Instance : public Object<O> {
			public:
				Instance(REFCLSID rclsid, DWORD dwClsContext, REFIID riid) {
					initialize();
					Win32::throw_if_fail(
						CoCreateInstance(
							rclsid,
							NULL,
							dwClsContext,
							riid,
							this->ptr()
						)
					);
				}

			};

		}

 	}

	namespace Network {


		namespace Connection {

			class UDJAT_API Manager {
			private:
				INetConnectionManager * manager = nullptr;

			public:
				Manager();
				~Manager();

				inline const INetConnectionManager * operator->() const {
					return manager;
				}

			};

			class UDJAT_API Properties {
			private:
				HMODULE netshell;
				IEnumNetConnection * pEnum = nullptr;
				INetConnection * pCon = nullptr;
				NETCON_PROPERTIES *properties = nullptr;

			public:
				Properties(const char *nicname);
				~Properties();

				inline const NETCON_PROPERTIES * operator->() const {
					return properties;
				}

			};

		}

	}

 }
