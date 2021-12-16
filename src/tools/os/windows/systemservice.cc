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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/systemservice.h>
 #include <iostream>
 #include <system_error>
 #include <udjat/tools/mainloop.h>

 using namespace std;

 namespace Udjat {

	namespace Win32 {

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
		struct Status : SERVICE_STATUS {

			constexpr Status() {
				dwCurrentState				= (DWORD) -1;
				dwWin32ExitCode				= 0;
				dwWaitHint					= 0;
				dwServiceType				= SERVICE_WIN32;
				dwServiceSpecificExitCode	= 0;
				dwControlsAccepted			= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
			}

			void set(SERVICE_STATUS_HANDLE handle, DWORD state, DWORD wait = 0) {

				if(state != dwCurrentState) {

					static const struct _state {
						DWORD state;
						const char *msg;
					} st[] = {
						{ SERVICE_START_PENDING,	"Service is pending start" },
						{ SERVICE_STOP_PENDING,		"Service is pending stop"	},
						{ SERVICE_STOPPED,			"Service is stopped"		},
						{ SERVICE_RUNNING,			"Service is running"		},
					};

					for(size_t f = 1; f < (sizeof(st)/sizeof(st[0]));f++) {
						if(st[f].state == state) {
							clog << "Service\t" << st[f].msg << endl;
							break;
						}
					}
				}

				dwCurrentState = state;
				dwWaitHint = wait;

				if(!SetServiceStatus(handle, (SERVICE_STATUS *) this)) {
					cerr << "Service\tSystem error in SetServiceStatus()" << endl;
				}

			}

		};

	}

	class Controller {
	private:
		Win32::Status status;
		SERVICE_STATUS_HANDLE hStatus = 0;
		SystemService *service = nullptr;

		Controller() {
		}

	public:
		static Controller & getInstance();

		~Controller() {
		}

		void set(SystemService *service) {

			if(this->service) {
				throw runtime_error(string{"Service is already registered as '"} + service->c_str() + "'");
			}

			this->service = service;
		}

		void set(DWORD state, DWORD wait) {
			status.set(hStatus,state,wait);
		}

		static void WINAPI handler( DWORD CtrlCmd ) {

			Controller &controller = getInstance();

			if(!controller.service) {
				// FIXME: Report failure.
				controller.set(controller.status.dwCurrentState, 0);
				return;
			}

			switch (CtrlCmd) {
			case SERVICE_CONTROL_SHUTDOWN:
					controller.set(SERVICE_STOP_PENDING, 3000);
					cout << "MainLoop\tSystem shutdown, stopping" << endl;
					controller.service->stop();
					break;

			case SERVICE_CONTROL_STOP:
					controller.set(SERVICE_STOP_PENDING, 3000);
					cout << "MainLoop\tStopping by request" << endl;
					controller.service->stop();
					break;

			case SERVICE_CONTROL_INTERROGATE:
					controller.set(controller.status.dwCurrentState, 0);
					break;

			default:
					clog << "MainLoop\tUnexpected service control code: " << ((int) CtrlCmd) << endl;
					controller.set(controller.status.dwCurrentState, 0);
			}

		}

		static void dispatcher() {

			Controller &controller = getInstance();

			// Inicia como serviço
			controller.hStatus = RegisterServiceCtrlHandler(TEXT(controller.service->c_str()), handler);

			if(!controller.hStatus) {
				throw runtime_error("RegisterServiceCtrlHandler failed");
			}

			try {

				controller.set(SERVICE_START_PENDING, 3000);
				controller.service->init();

				controller.set(SERVICE_RUNNING, 0);
				controller.service->run();

				controller.set(SERVICE_STOP_PENDING, 3000);
				controller.service->deinit();

			} catch(const std::exception &e) {

				cerr << "MainLoop\t" << e.what() << endl;

			} catch(...) {

				cerr << "MainLoop\tUnexpected error starting windows service" << endl;
			}

			controller.set(SERVICE_STOPPED, 0);

		}

	};

	Controller & Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	SystemService::SystemService(const char *n) : name(n) {
		Controller::getInstance().set(this);
	}

	SystemService::~SystemService() {
	}

	void SystemService::init() {
	}

	void SystemService::run() {
		MainLoop::getInstance().run();
	}

	void SystemService::deinit() {
	}

	void SystemService::start() {

		static SERVICE_TABLE_ENTRY DispatchTable[] = {
			{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) Controller::dispatcher },
			{ NULL, NULL }
		};

		DispatchTable[0].lpServiceName = TEXT( (char *) name);

		cout << name << "\tStarting service dispatcher" << endl;

		if(!StartServiceCtrlDispatcher( DispatchTable )) {
			cerr << name << "\tError in service dispatcher" << endl;
		}

	}

	void SystemService::stop() {
		MainLoop::getInstance().quit();
	}

 }

