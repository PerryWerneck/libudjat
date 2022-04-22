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
 #include <udjat/win32/registry.h>
 #include <udjat/tools/logger.h>
 #include <udjat/agent.h>
 #include <udjat/module.h>
 #include <direct.h>

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
							clog << "service\t" << st[f].msg << endl;
							break;
						}
					}
				}

				dwCurrentState = state;
				dwWaitHint = wait;

				if(!SetServiceStatus(handle, (SERVICE_STATUS *) this)) {
					cerr << "service\tWindows error " << GetLastError() << " in SetServiceStatus(" << state << ")" << endl;
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

			Controller() {
			}

			void set(DWORD state, DWORD wait = 0) {
				status.set(hStatus,state,wait);
			}

		public:
			static Controller & getInstance();

			~Controller() {
			}

			static void WINAPI handler( DWORD CtrlCmd ) {

				Controller &controller = getInstance();

				if(!SystemService::getInstance()) {
					// FIXME: Report failure.
					controller.set(controller.status.dwCurrentState, 0);
					return;
				}

				try {

					switch (CtrlCmd) {
					case SERVICE_CONTROL_SHUTDOWN:
						controller.set(SERVICE_STOP_PENDING, 3000);
						cout << "service\tSystem shutdown, stopping" << endl;
						SystemService::getInstance()->stop();
						break;

					case SERVICE_CONTROL_STOP:
						controller.set(SERVICE_STOP_PENDING, 3000);
						cout << "service\tStopping by request" << endl;
						SystemService::getInstance()->stop();
						break;

					case SERVICE_CONTROL_INTERROGATE:
						controller.set(controller.status.dwCurrentState, 0);
						break;

					default:
						clog << "service\tUnexpected win32 service control code: " << ((int) CtrlCmd) << endl;
						controller.set(controller.status.dwCurrentState, 0);
					}

				} catch(const std::exception &e) {

					cerr << "service\tError '" << e.what() << "' handling service request" << endl;
					// FIXME: Report failure.
					controller.set(controller.status.dwCurrentState, 0);

				} catch(...) {

					cerr << "service\tUnexpected error handling service request" << endl;
					// FIXME: Report failure.
					controller.set(controller.status.dwCurrentState, 0);

				}

			}

			static void dispatcher() {

				Controller &controller = getInstance();

				// Inicia como serviço
				controller.hStatus = RegisterServiceCtrlHandler(TEXT(Application::Name::getInstance().c_str()),handler);
				if(!controller.hStatus) {
					cerr << "service\tRegisterServiceCtrlHandler failed with windows error " << GetLastError() << endl;
					return;
				}

				auto service = SystemService::getInstance();

				if(service) {

					try {

						controller.set(SERVICE_START_PENDING, 3000);
						service->init();

						controller.set(SERVICE_RUNNING, 0);
						service->run();

					} catch(const std::exception &e) {

						Application::error() << "Error '" << e.what() << "' running service" << endl;

					} catch(...) {

						Application::error() << "Unexpected error running service" << endl;

					}

					controller.set(SERVICE_STOP_PENDING, 3000);
					service->deinit();
				}

				controller.set(SERVICE_STOPPED, 0);

			}

		};

		Controller & Controller::getInstance() {
			static Controller instance;
			return instance;
		}

	}

	void SystemService::init() {

		setlocale( LC_ALL, "" );

		Module::load();

		if(definitions) {
			reconfigure(definitions,true);
		}

	}

	void SystemService::notify(const char *message) noexcept {

		try {

			Win32::Registry registry("service",true);

			registry.set("status",message);
			registry.set("status_time",TimeStamp().to_string().c_str());

			info() << message << endl;

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' setting service state" << endl;

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

	void SystemService::usage() const noexcept {

		cout << "Usage: " << endl << endl << "  ";

		TCHAR filename[MAX_PATH];
		if(GetModuleFileName(NULL, filename, MAX_PATH ) ) {
			cout << filename;
		} else {
			cout << name();
		}

		cout	<< " [options]" << endl << endl
				<< "  --foreground\t\tRun " << name() << " service as application (foreground)" << endl
				<< "  --start\t\tStart " << name() << " service" << endl
				<< "  --stop\t\tStop " << name() << " service" << endl
				<< "  --restart\t\tRestart " << name() << " service" << endl
				<< "  --install\t\tInstall " << name() << " service" << endl
				<< "  --uninstall\t\tUninstall " << name() << " service" << endl;
	}

	static int service_start(const char *appname) {

		Win32::Service::Manager manager;
		Win32::Service::Handler service{manager.open(appname)};
		if(!service) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "winservice\tService '" << appname << "' does not exist" << endl;
				return -1;
			}
			throw Win32::Exception("Cant stop service",lasterror);
		}

		try {

			clog << "winservice\tStarting service '" << appname << "'" << endl;
			service.start();
			clog << "winservice\tService '" << appname << "' was started" << endl;

		} catch(const exception &e) {
			cerr << e.what() << endl;
			return -1;
		}

		return 0;

	}

	static int service_stop(const char *appname) {

		Win32::Service::Manager manager;
		Win32::Service::Handler service{manager.open(appname)};
		if(!service) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "winservice\tService '" << appname << "' does not exist" << endl;
				return -1;
			}
			throw Win32::Exception("Cant stop service",lasterror);
		}

		try {

			service.stop();

		} catch(const exception &e) {

			cerr << "winservice\t" << e.what() << endl;
			return -1;

		}

		return 0;

	}

	int SystemService::cmdline(char key, const char UDJAT_UNUSED(*value)) {

		switch(key) {
		case 'i':	// Install service.
			Logger::redirect(true);
			return install();

		case 's':	// Start service.
			Logger::redirect(true);
			return service_start(name().c_str());

		case 'r':	// Restart service.
			Logger::redirect(true);
			service_stop(name().c_str());
			service_start(name().c_str());
			return 0;

		case 'R':	// Reinstall service.
			Logger::redirect(true);
			service_stop(name().c_str());
			uninstall();
			install();
			service_start(name().c_str());
			return 0;

		case 'q':	// Stop service.
			Logger::redirect(true);
			return service_stop(name().c_str());

		case 'u':	// Uninstall service.
			Logger::redirect(true);
			return uninstall();

		case 'f':	// Run in foreground.
			cout << "Starting " << name() << " application" << endl << endl;

			Logger::redirect(true);

			try {

				init();
				run();

			} catch(const std::exception &e) {

				cerr << name() << "\tError '" << e.what() << "' running application" << endl;

			} catch(...) {

				cerr << name() << "\tUnexpected error running application" << endl;

			}

			deinit();

			return 0;

		}

		return ENOENT;
	}

	int SystemService::cmdline(const char *key, const char *value) {

		// The default options doesn't have values, then, reject here.
		if(value) {
			return ENOENT;
		}

		static const struct {
			char option;
			const char *key;
		} options[] = {
			{ 'i', "install" },
			{ 'u', "uninstall" },
			{ 's', "start" },
			{ 'q', "stop" },
			{ 'r', "restart" },
			{ 'R', "reinstall" },
			{ 'f', "foreground" }
		};

		for(size_t option = 0; option < (sizeof(options)/sizeof(options[0])); option++) {
			if(!strcasecmp(key,options[option].key)) {
				return cmdline(options[option].option);
			}
		}

		return ENOENT;
	}

	int SystemService::run(int argc, char **argv) {

		{
			WSADATA WSAData;
			WSAStartup(0x101, &WSAData);

			// https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-/blob/master/docs/part-04.md
			SetConsoleOutputCP(CP_UTF8);
			SetConsoleCP(CP_UTF8);

			_chdir(Application::Path().c_str());
		}

		auto appname = Application::Name::getInstance();

		if(argc > 1) {
			return cmdline(argc,(const char **) argv);
		}

		// Redirect output
		cout << "Starting service dispatcher" << endl;
		Logger::redirect(false);

		// Run as service by default.
		static SERVICE_TABLE_ENTRY DispatchTable[] = {
			{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) Service::Controller::dispatcher },
			{ NULL, NULL }
		};

		DispatchTable[0].lpServiceName = TEXT( (char *) appname.c_str());

		cout << "Starting " << appname << " service dispatcher" << endl;

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

		cout << "winservice\tInserting '" << display_name << "' service" << endl;
#ifdef DEBUG
		cout << "Service binary is '" << service_binary << "'" << endl;
#endif // DEBUG

		manager.insert(appname.c_str(),display_name,service_binary);
		cout << "winservice\tService '" << display_name << "' installed" << endl;

		/*
		// Stop previous instance
		try {

			cout << appname << "\tStopping previous instance" << endl;
			Win32::Service::Handler(manager.open(appname.c_str())).stop();
			cout << appname << "\tPrevious instance stopped" << endl;

		} catch(const exception &e) {
			cerr << appname << "\t" << e.what() << endl;
		}

		// Remove previous instance
		try {

			cout << appname << "\tRemoving previous instance" << endl;
			manager.remove(appname.c_str());
			cout << appname << "\tPrevious instance removed" << endl;

		} catch(const exception &e) {
			cerr << appname << "\t" << e.what() << endl;
		}

		// Inclui novo serviço
		cout << appname << "\tInserting '" << display_name << "' service" << endl;
#ifdef DEBUG
		cout << appname << "\tService binary is '" << service_binary << "'" << endl;
#endif // DEBUG
		manager.insert(appname.c_str(),display_name,service_binary);
		cout << appname << "\tService '" << display_name << "' inserted" << endl;
		*/

		return 0;

	}

	/// @brief Uninstall win32 service.
	int SystemService::uninstall() {

		Application::Name appname;

		Win32::Service::Manager manager;

		if(!Win32::Service::Handler(manager.open(appname.c_str()))) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "winservice\tService '" << appname << "' does not exist" << endl;
				return 0;
			}
			throw Win32::Exception("Cant open service",lasterror);
		}

		try {

			cout << "winservice\tRemoving service '" << appname << "'" << endl;
			manager.remove(appname.c_str());
			cout << "winservice\tService '" << appname << "' removed" << endl;

		} catch(const exception &e) {
			cerr << "winservice\tCant remove '" << appname << "': " << e.what() << endl;
			return -1;
		}

		return 0;
	}

 }

