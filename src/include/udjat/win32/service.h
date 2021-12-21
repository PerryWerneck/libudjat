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

 #ifndef _WIN32
	#error Service objects requires win32
 #endif // _WIN32

 #include <stdexcept>
 #include <aclapi.h>
 #include <udjat/win32/security.h>
 #include <udjat/win32/exception.h>
 #include <functional>

 namespace Udjat {

	namespace Win32 {

		class UDJAT_API UDJAT_API Service {
		private:
			SC_HANDLE handle;

		public:

			class UDJAT_API Security {
			private:
				SC_HANDLE handle;
				PSECURITY_DESCRIPTOR	pSD;

			public:
				Security(const Service &service);
				~Security();

				/// @brief Aplica ACLs no objeto.
				void set(PEXPLICIT_ACCESS acl, size_t cCountOfExplicitEntries);

				/// @brief Apply ACLs to object.
				/// @param permissions	Array with the permissions to set.
				/// @param length		Length of the permissions array.
				void set(const Permission *permissions, size_t length);

				/// @brief Aplica permissões de segurança no serviço.
				void setProtected();

			};

			Service() : handle(0) {
			}

			Service(SC_HANDLE h) : handle(h) {
			}

			Service & operator =(SC_HANDLE h) noexcept {
				if(handle) {
					CloseServiceHandle(handle);
				}
				handle = h;
				return *this;
			}

			~Service() {
				if(handle) {
					CloseServiceHandle(handle);
				}
			}

			inline SC_HANDLE getHandle() const noexcept {
				return handle;
			}

			/// @brief Start service
			void start() {
				if(!StartService(handle,0,NULL)) {
					throw Win32::Exception("Can't start service");
				}
			}

			/// @brief Stop service.
			void stop(bool wait = true);

			/// @brief Install service.
			/// @param name Service name.
			/// @param service_binary Full path to the service application.
			/// @param start True to start service after installation.
			int install(const char *name, const char *display_name, const char *service_binary, bool start = true);

		};


		/// @brief Windows Service Manager.
		class UDJAT_API ServiceManager {
		private:
			SC_HANDLE handle;

		public:
			ServiceManager(DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS) {
				handle = OpenSCManager(NULL, NULL, dwDesiredAccess);
				if(!handle) {
					throw Win32::Exception("Can't open windows service manager");
				}
			}

			~ServiceManager() {
				CloseServiceHandle(handle);
			}

			/// @brief Open Service.
			SC_HANDLE open(const char *name, DWORD dwDesiredAccess = SERVICE_ALL_ACCESS) {
				SC_HANDLE hService = OpenService(handle, name, dwDesiredAccess);
				if(!hService) {
					throw Win32::Exception("Can't open service");
				}
				return hService;
			}

			/// @brief Create windows service.
			SC_HANDLE insert(const char *name, const char *display_name, const char *service_binary) {

				// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682450(v=vs.85).aspx
				SC_HANDLE hService = CreateService(	handle,
													TEXT(name),	 							// Service name
													TEXT(display_name),						// Service display name
													SERVICE_ALL_ACCESS,						// Includes STANDARD_RIGHTS_REQUIRED in addition to all access rights in this table.
													SERVICE_WIN32_OWN_PROCESS,				// Service that runs in its own process..
													SERVICE_AUTO_START,						// A service started automatically by the service control manager during system startup.
													SERVICE_ERROR_NORMAL,					// The severity of the error, and action taken, if this service fails to start.
													service_binary,							// The fully-qualified path to the service binary file.
													NULL,									// Load order group
													NULL,									// Group member tab
													NULL,									// Dependencies
													NULL,									// Account
													NULL									// Password
												);

				if(!hService) {
					throw Win32::Exception("Can't create service");
				}

				return hService;
			}

			/// @brief Remove service
			void remove(const char *name) {

				SC_HANDLE hService = OpenService(handle, TEXT(name), SERVICE_ALL_ACCESS);
				if(!hService) {
					if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST) {
						return;
					}
					throw Win32::Exception("Can't open service");
				}

				if(!DeleteService(hService)) {

					// Não consegui remover o serviço
					CloseServiceHandle(hService);
					throw std::runtime_error("Can't delete service");

				}

				CloseServiceHandle(hService);

			}

		};

	}

 }
