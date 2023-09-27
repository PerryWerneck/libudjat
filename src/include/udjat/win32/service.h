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

		namespace Service {

			///
			/// @brief Contains status information for a service.
			///
			/// The ControlService, EnumDependentServices, EnumServicesStatus, and QueryServiceStatus
			/// functions use this structure.
			///
			/// A service uses this structure in the SetServiceStatus function to report its
			/// current status to the service control manager.
			///
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms685996(v=vs.85).aspx
			///
			struct UDJAT_API Status : SERVICE_STATUS {

				constexpr Status() {

					/// @brief The type of service.
					dwServiceType				= SERVICE_WIN32;

					/// @brief The current state of the service.
					dwCurrentState				= SERVICE_STOPPED;

					// A service cant accept SERVICE_ACCEPT_SHUTDOWN and SERVICE_ACCEPT_PRESHUTDOWN, just one of them.
					// From http://msdn.microsoft.com/en-us/library/windows/desktop/ms683241(v=vs.85).aspx
					//
					// Referring to SERVICE_CONTROL_PRESHUTDOWN:
					//
					// A service that handles this notification blocks system shutdown until the service stops
					// or the preshutdown time-out interval specified through SERVICE_PRESHUTDOWN_INFO expires.
					//
					// In the same page, the section about SERVICE_CONTROL_SHUTDOWN adds:
					//
					// Note that services that register for SERVICE_CONTROL_PRESHUTDOWN notifications cannot
					// receive this notification because they have already stopped.
					//
					// So, the correct way is to set the dwControlsAccepted to include either SERVICE_ACCEPT_SHUTDOWN
					// or SERVICE_ACCEPT_PRESHUTDOWN, depending on your needs, but not to both at the same time.
					//
					// But do note that you probably want to accept more controls. You should always allow at least
					// SERVICE_CONTROL_INTERROGATE, and almost certainly allow SERVICE_CONTROL_STOP, s
					// ince without the latter the service cannot be stopped (e.g. in order to uninstall the software)
					// and the process will have to be forcibly terminated (i.e. killed).

					/// @brief The control codes the service accepts and processes in its handler function
					dwControlsAccepted			=
						SERVICE_ACCEPT_STOP |
#ifdef SERVICE_ACCEPT_PRESHUTDOWN
						SERVICE_ACCEPT_PRESHUTDOWN |
#endif // SERVICE_ACCEPT_PRESHUTDOWN
						SERVICE_ACCEPT_POWEREVENT |
						SERVICE_ACCEPT_SESSIONCHANGE;

					/// @brief The error code the service uses to report an error that occurs when it is starting or stopping.
					dwWin32ExitCode				= NO_ERROR;

					/// @brief A service-specific error code that the service returns when an error occurs while the service is starting or stopping.
					dwServiceSpecificExitCode	= 0;

					/// @brief The check-point value the service increments periodically to report its progress during a lengthy start, stop, pause, or continue operation.
					dwCheckPoint				= 0;

					/// @brief The estimated time required for a pending start, stop, pause, or continue operation, in milliseconds.
					dwWaitHint					= 0;

				}

				void set(SERVICE_STATUS_HANDLE handle, DWORD state, DWORD wait = 0) noexcept;

			};

			/// @brief Windows Service Manager.
			class UDJAT_API Manager {
			private:
				SC_HANDLE handle;	///< @brief Handle to the service control manager database.

				/// @brief Open Service.
				SC_HANDLE open(const char *name, DWORD dwDesiredAccess = SERVICE_ALL_ACCESS);

			public:
				Manager(const Manager &m) = delete;
				Manager(const Manager *m) = delete;

				Manager(DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS);
				~Manager();

				/// @brief Create windows service.
				void insert(const char *name, const char *display_name, const char *service_binary);

				/// @brief Remove windows service.
				/// @param name The service name.
				/// @retval true The service was removed.
				/// @retval false The service was not found.
				bool remove(const char *name);

				/// @brief Start windows service.
				/// @param name The service name.
				/// @retval true The service was started.
				/// @retval false The service was not found.
				bool start(const char *name);

				/// @brief Stop windows service.
				/// @param name The service name.
				/// @retval true The service was stopped.
				/// @retval false The service was not found.
				bool stop(const char *name);
			};

			class UDJAT_API Handler {
			private:
				SC_HANDLE handle;

			public:
				Handler(const Handler &h) = delete;
				Handler(const Handler *h) = delete;

//				Handler(const char *name);

				constexpr Handler() : handle(0) {
				}

				constexpr Handler(SC_HANDLE h) : handle(h) {
				}

				inline operator bool() const noexcept {
					return handle != NULL;
				}

				Handler & operator = (SC_HANDLE h) noexcept {
					if(handle) {
						CloseServiceHandle(handle);
					}
					handle = h;
					return *this;
				}

				~Handler() {
					if(handle) {
						CloseServiceHandle(handle);
					}
				}

				inline SC_HANDLE getHandle() const noexcept {
					return handle;
				}

				/// @brief Start service
				void start();

				/// @brief Stop service.
				void stop(bool wait = true);
			};

		}

	}

 }
