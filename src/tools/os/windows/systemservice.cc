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
 #include <udjat/tools/application.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/service.h>
 #include <udjat/tools/logger.h>
 #include <getopt.h>

 using namespace std;

 namespace Udjat {

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

		/// @brief WIN32 Service controller.
		///
		/// Associate an udjat systemservice with the win32 service dispatcher.
		///
		class UDJAT_API Controller {
		private:
			Status status;
			SERVICE_STATUS_HANDLE hStatus = 0;
			Udjat::SystemService *service = nullptr;

			Controller() {
			}

			void set(DWORD state, DWORD wait = 0) {
				status.set(hStatus,state,wait);
			}

		public:
			static Controller & getInstance();

			~Controller() {
			}

			void insert(SystemService *service) {

				if(this->service) {
					throw runtime_error("WIN32 System Service is already registered");
				}

				this->service = service;
			}

			void remove(SystemService *service) {
				if(this->service == service) {
					this->service = nullptr;
				}
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
						cout << "service\tSystem shutdown, stopping" << endl;
						controller.service->stop();
						break;

				case SERVICE_CONTROL_STOP:
						controller.set(SERVICE_STOP_PENDING, 3000);
						cout << "service\tStopping by request" << endl;
						controller.service->stop();
						break;

				case SERVICE_CONTROL_INTERROGATE:
						controller.set(controller.status.dwCurrentState, 0);
						break;

				default:
						clog << "service\tUnexpected win32 service control code: " << ((int) CtrlCmd) << endl;
						controller.set(controller.status.dwCurrentState, 0);
				}

			}

			static void dispatcher() {

				Controller &controller = getInstance();

				// Inicia como serviço
				controller.hStatus = RegisterServiceCtrlHandler(TEXT(Application::Name().c_str()), handler);

				if(!controller.hStatus) {
					throw runtime_error("RegisterServiceCtrlHandler failed");
				}

				try {

					Logger::redirect();

					controller.set(SERVICE_START_PENDING, 3000);
					controller.service->init();

					controller.set(SERVICE_RUNNING, 0);
					controller.service->run();

					controller.set(SERVICE_STOP_PENDING, 3000);
					controller.service->deinit();

				} catch(const std::exception &e) {

					cerr << "service\t" << e.what() << endl;

				} catch(...) {

					cerr << "service\tUnexpected error starting windows service" << endl;
				}

				controller.set(SERVICE_STOPPED, 0);

			}

		};

		Controller & Controller::getInstance() {
			static Controller instance;
			return instance;
		}

	}

	SystemService::SystemService() {
		setlocale( LC_ALL, "" );
		Service::Controller::getInstance().insert(this);
	}

	SystemService::~SystemService() {
		Service::Controller::getInstance().remove(this);
	}

	void SystemService::init() {
		if(definitions) {
			cout << Application::Name() << "\tInitializing from '" << definitions << "'" << endl;
			Udjat::load(definitions);
		}
	}

	void SystemService::deinit() {
	}

	void SystemService::stop() {
		MainLoop::getInstance().quit();
	}

	int SystemService::run() {
		MainLoop::getInstance().run();
		return 0;
	}

	void SystemService::usage(const char *appname) const noexcept {
		cout 	<< "Usage: " << endl << endl << "  " << appname << " [options]" << endl << endl
				<< "  --foreground\t\tRun " << appname << " service as application (foreground)" << endl
				<< "  --install\t\tInstall " << appname << " service" << endl
				<< "  --install-and-start\tInstall " << appname << " service and start it" << endl
				<< "  --start\t\tStart " << appname << " service" << endl
				<< "  --stop\t\tStop " << appname << " service" << endl
				<< "  --uninstall\t\tUninstall " << appname << " service" << endl
				<< endl;
	}

	static int service_start(const char *appname) {

		Win32::Service::Manager manager;

		try {

			Win32::Service::Handler(manager.open(appname)).start();

		} catch(const exception &e) {
			cerr << e.what() << endl;
			return -1;
		}

		return 0;

	}

	static int service_stop(const char *appname) {

		Win32::Service::Manager manager;

		try {

			Win32::Service::Handler(manager.open(appname)).stop();

		} catch(const exception &e) {
			cerr << e.what() << endl;
			return -1;
		}

		return 0;

	}

	int SystemService::run(int argc, char **argv) {

		{
			WSADATA WSAData;
			WSAStartup(0x101, &WSAData);

			// https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-/blob/master/docs/part-04.md
			SetConsoleOutputCP(CP_UTF8);
			SetConsoleCP(CP_UTF8);
		}

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
		static struct option options[] = {
			{ "foreground",			no_argument,		0,	'f' },
			{ "install",			no_argument,		0,	'i' },
			{ "install-and-start",	no_argument,		0,	'I' },
			{ "start",				no_argument,		0,	's' },
			{ "stop",				no_argument,		0,	'q' },
			{ "uninstall",			no_argument,		0,	'u' },
			{ "help",				no_argument,		0,	'h' },
			{ NULL }
		};
		#pragma GCC diagnostic pop

		auto appname = Application::Name::getInstance();
		int rc = 0;

		// Parse command line options.
		{
			int long_index =0;
			int opt;
			while((opt = getopt_long(argc, argv, "fiIsquh", options, &long_index )) != -1) {
				try {

					switch(opt) {
					case 'h':
						usage(appname.c_str());
						return 0;

					case 'i':	// Install service.
						return install();

					case 's':	// Start service.
						return service_start(appname.c_str());

					case 'q':	// Stop service.
						return service_stop(appname.c_str());

					case 'I':	// Install and start service.
						if(!install()) {
							return service_start(appname.c_str());
						}
						return -1;

					case 'u':	// Uninstall service.
						return uninstall();

					case 'f':	// Run in foreground.
						Logger::redirect(nullptr,true);
						cout << appname << "\tStarting in application mode" << endl;
						init();
						rc = run();
						deinit();
						return rc;
						break;

					}

				} catch(const std::exception &e) {
					cerr << appname << "\t" << e.what() << endl;
					return -1;

				} catch(...) {
					cerr << appname << "\tUnexpected error" << endl;
					return -1;

				}

			}

		}

		// Run as service by default.
		static SERVICE_TABLE_ENTRY DispatchTable[] = {
			{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) Service::Controller::dispatcher },
			{ NULL, NULL }
		};

		DispatchTable[0].lpServiceName = TEXT( (char *) appname.c_str());

		cout << "Starting '" << appname << "' service dispatcher" << endl;

		if(!StartServiceCtrlDispatcher( DispatchTable )) {
			cerr << "Failed to start '" << appname << "' service dispatcher" << endl << Win32::Exception::format(GetLastError()) << endl;
			return -1;
		}

		return 0;

	}

	int SystemService::install() {
		return install(Application::Name().c_str());
	}

	/// @brief Install win32 service.
	int SystemService::install(const char *display_name) {

		Application::Name appname;

		// Get my path
		TCHAR service_binary[MAX_PATH];
		if(!GetModuleFileName(NULL, service_binary, MAX_PATH ) ) {
			throw Win32::Exception("Can't get service filename");
		}

		Win32::Service::Manager manager;

		try {

			cout << appname << "\tStopping previous instance" << endl;
			Win32::Service::Handler(manager.open(appname.c_str())).stop();
			cout << appname << "\tPrevious instance stopped" << endl;

		} catch(const exception &e) {
			cerr << appname << "\t" << e.what() << endl;
		}

		try {

			cout << appname << "\tRemoving previous instance" << endl;
			manager.remove(appname.c_str());
			cout << appname << "\tPrevious instance removed" << endl;

		} catch(const exception &e) {
			cerr << appname << "\t" << e.what() << endl;
		}

		// Inclui novo serviço
		cout << appname << "\tInserting '" << display_name << "' service" << endl;
		manager.insert(appname.c_str(),display_name,service_binary);
		cout << appname << "\tService '" << display_name << "' inserted" << endl;

		return 0;

	}

	/// @brief Uninstall win32 service.
	int SystemService::uninstall() {

		Application::Name appname;

		Win32::Service::Manager manager;

		try {

			cout << appname << "\tStopping previous instance" << endl;
			Win32::Service::Handler(manager.open(appname.c_str())).stop();
			cout << appname << "\tPrevious instance stopped" << endl;

		} catch(const exception &e) {
			cerr << appname << "\t" << e.what() << endl;
		}

		try {

			cout << appname << "\tRemoving previous instance" << endl;
			manager.remove(appname.c_str());
			cout << appname << "\tPrevious instance removed" << endl;

		} catch(const exception &e) {
			cerr << appname << "\t" << e.what() << endl;
			return -1;
		}

		return 0;
	}

 }

