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
 #include <udjat/tools/configuration.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/service.h>
 #include <udjat/win32/registry.h>
 #include <udjat/tools/logger.h>
 #include <udjat/agent.h>
 #include <udjat/module.h>
 #include <direct.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/file.h>

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

				Logger::redirect(false);

				Controller &controller = getInstance();

				// Inicia como serviço
				controller.hStatus = RegisterServiceCtrlHandler(TEXT(Application::Name::getInstance().c_str()),handler);
				if(!controller.hStatus) {
					cerr << "service\tRegisterServiceCtrlHandler failed with windows error " << GetLastError() << endl;
					return;
				}

				auto service = SystemService::getInstance();

				if(service) {

					service->info() << "Running as windows service" << endl;

					try {

						controller.set(SERVICE_START_PENDING, 3000);
						service->init();

						controller.set(SERVICE_RUNNING, 0);
						service->run();

					} catch(const std::exception &e) {

						service->error() << "Error '" << e.what() << "' running service" << endl;

					} catch(...) {

						service->error() << "Unexpected error running service" << endl;

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

		string appconfig;
		if(!definitions) {
			Config::Value<string> config("service","definitions","");
			if(config.empty()) {
				definitions = Quark(Application::DataDir("xml.d")).c_str();
			} else {
				definitions = Quark(config).c_str();
			}
		}

		if(!Module::preload(definitions)) {
			throw runtime_error("Module preload has failed, aborting service");
		}

		if(definitions[0] && strcasecmp(definitions,"none")) {

			info() << "Loading service definitions from " << definitions << endl;

			reconfigure(definitions,true);

		}

	}

	void SystemService::registry(const char *name, const char *value) {

		try {

			Win32::Registry("service",true).set(name,value);

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' while updating registry service/" << name << "=" << value << endl;

		} catch(...) {

			error() << "Unexpected error while updating registry service/" << name << "=" << value << endl;

		}
	}

	void SystemService::notify(const char *message) noexcept {

		if(!*message) {
			return;
		}

		try {

			Win32::Registry registry("service",true);

			registry.set("status",message);
			registry.set("status_time",TimeStamp().to_string().c_str());

			info() << message << endl;

			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

			if(hOut != INVALID_HANDLE_VALUE) {
				DWORD mode = 0;
				if(GetConsoleMode(hOut, &mode) && (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
					DWORD dunno;

					// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences

					WriteFile(hOut,"\x1b]0;",3,&dunno,NULL);
					WriteFile(hOut,message,strlen(message),&dunno,NULL);
					WriteFile(hOut,"\x1b\x5c",2,&dunno,NULL);
				}
			}


		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' setting service state" << endl;

		}

	}

	void SystemService::deinit() {
	}

	void SystemService::stop() {
		notify("Stopping");
		MainLoop::getInstance().quit();
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
				<< "  --timer=seconds\tTerminate " << name() << " after 'seconds'" << endl
				<< "  --start\t\tStart " << name() << " service" << endl
				<< "  --stop\t\tStop " << name() << " service" << endl
				<< "  --restart\t\tRestart " << name() << " service" << endl
				<< "  --install\t\tInstall " << name() << " service" << endl
				<< "  --uninstall\t\tUninstall " << name() << " service" << endl;
	}

	/// @brief Start service by name.
	/// @param appname Service name.
	/// @retval ENOENT Service does not exist.
	/// @retval 0 Service was started.
	/// @retval -1 Unexpected error.
	static int service_start(const char *appname) {

		cout << "win32\tStarting service '" << appname << "'" << endl;

		Win32::Service::Manager manager;
		Win32::Service::Handler service{manager.open(appname)};
		if(!service) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "winservice\tService '" << appname << "' does not exist" << endl;
				return ENOENT;
			}
			throw Win32::Exception("Cant start service",lasterror);
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

	/// @brief Stop win32 service.
	/// @retval 0 Services was stopped or does not exist.
	/// @retval -1 Unexpected error.
	static int service_stop(const char *appname) {

		cout << "win32\tStopping service '" << appname << "'" << endl;

		Win32::Service::Manager manager;
		Win32::Service::Handler service{manager.open(appname)};
		if(!service) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "winservice\tService '" << appname << "' does not exist" << endl;
				return 0;
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

	/// @brief Remove registry
	static int reset_to_defaults() {

		cout << "win32\tCleaning application registry" << endl;

		static const DWORD options[] = { KEY_ALL_ACCESS|KEY_WOW64_32KEY, KEY_ALL_ACCESS|KEY_WOW64_64KEY };
		for(size_t ix = 0; ix < (sizeof(options)/sizeof(options[0])); ix++) {

			HKEY hKey = 0;
			LSTATUS rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE"),0,NULL,REG_OPTION_NON_VOLATILE,options[ix],NULL,&hKey,NULL);

			if(rc == ERROR_SUCCESS) {

				Win32::Registry{hKey}.remove(Application::Name().c_str());

			} else if(rc != ERROR_FILE_NOT_FOUND) {

				trace("RegCreateKeyEx(HKLM/SOFTWARE) rc was ",rc);
				throw Win32::Exception("Cant open application registry",rc);

			}

		}

		trace("Registry cleanup complete");
		return 0;
	}

	int SystemService::cmdline(char key, const char *value) {

		switch(key) {
		case 'T':	// Auto quit

			{

				if(!value) {
					throw system_error(EINVAL,system_category(),_( "Invalid timer value" ));
				}

				int seconds = atoi(value);
				if(!seconds) {
					throw system_error(EINVAL,system_category(),_( "Invalid timer value" ));
				}

				trace("Auto close timer set to ",seconds);

				MainLoop::getInstance().TimerFactory(seconds * 1000,[](){
					Application::warning() << "Exiting by timer request" << endl;
					MainLoop::getInstance().quit();
					return false;
				});

			}
			return 0;

		case 'C':	// Reset to defaults.
			Logger::redirect(true);
			mode = SERVICE_MODE_NONE;
			return reset_to_defaults();

		case 'i':	// Install service.
			Logger::redirect(true);
			mode = SERVICE_MODE_NONE;
			return install();

		case 's':	// Start service.
			Logger::redirect(true);
			mode = SERVICE_MODE_NONE;
			return service_start(name().c_str());

		case 'r':	// Restart service.
			Logger::redirect(true);
			mode = SERVICE_MODE_NONE;
			service_stop(name().c_str());
			service_start(name().c_str());
			return 0;

		case 'R':	// Reinstall service.
			Logger::redirect(true);
			mode = SERVICE_MODE_NONE;
			service_stop(name().c_str());
			uninstall();
			install();
			service_start(name().c_str());
			return 0;

		case 'q':	// Stop service.
			Logger::redirect(true);
			mode = SERVICE_MODE_NONE;
			return service_stop(name().c_str());

		case 'u':	// Uninstall service.
			Logger::redirect(true);
			mode = SERVICE_MODE_NONE;
			return uninstall();

		}

		return ENOENT;
	}

	int SystemService::cmdline(const char *key, const char *value) {

		static const struct {
			char option;
			const char *key;
		} options[] = {
			{ 'i', "install" },
			{ 'T', "timer" },
			{ 'u', "uninstall" },
			{ 's', "start" },
			{ 'q', "stop" },
			{ 'r', "restart" },
			{ 'R', "reinstall" },
			{ 'C', "reset-to-defaults" },
		};

		if(!strcasecmp(key,"force-uninstall")) {

			Logger::redirect(true,false);
			mode = SERVICE_MODE_NONE;

			Application::InstallLocation instpath;

#ifdef DEBUG
			instpath.assign(Application::Path());
#endif // DEBUG

			if(!instpath) {
				error() << "This application was not installed" << endl;
				return 1;
			}

			info() << "Doing a forced uninstall" << endl;

			// Stop service
			if(service_stop(name().c_str())) {
				return 2;
			}

			// Uninstall service
			if(uninstall()) {
				return 3;
			}

			// Remove registry entries.
			if(reset_to_defaults()) {
				return 4;
			}

			// Remove from system registry.
			static const DWORD options[] = { KEY_ALL_ACCESS|KEY_WOW64_32KEY, KEY_ALL_ACCESS|KEY_WOW64_64KEY };
			for(size_t ix = 0; ix < (sizeof(options)/sizeof(options[0])); ix++) {

				HKEY hKey;
				LSTATUS rc =
					RegOpenKeyEx(
						HKEY_LOCAL_MACHINE,
						TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
						0,
						options[ix],
						&hKey
					);

				if(rc == ERROR_SUCCESS) {

					Win32::Registry{hKey}.remove(Application::Name().c_str());

				} else if(rc != ERROR_FILE_NOT_FOUND) {

					cerr << "win32\t" << Win32::Exception::format("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall") << endl;

				}

			}

			// Remove all files from instpath.
			{

				cout << "win32\tRemoving files in '" << instpath << "'" << endl;

				File::Path::for_each(instpath.c_str(),"*",true,[this](bool isdir, const char *path){
					// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-deletefile
					trace("Removing '",path,"'");
					if(isdir) {
						if(RemoveDirectory(path) == 0) {
							auto rc = GetLastError();
							if(rc == ERROR_DIR_NOT_EMPTY) {
								MoveFileEx(path,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
							} else {
								error() << Win32::Exception::format(path,rc) << endl;
							}
						}
					} else if(DeleteFile(path) == 0) {
						auto rc = GetLastError();
						if(rc == ERROR_ACCESS_DENIED) {
							MoveFileEx(path,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
						} else {
							error() << Win32::Exception::format(path,rc) << endl;
						}
					}
					return true;
				});

				MoveFileEx(instpath.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);

			}

			return 0;
		}

		for(size_t option = 0; option < (sizeof(options)/sizeof(options[0])); option++) {
			if(!strcasecmp(key,options[option].key)) {
				return cmdline(options[option].option,value);
			}
		}

		return ENOENT;
	}

	int SystemService::run(int argc, char **argv) {

		int rc = 0;

		auto appname = Application::Name::getInstance();

		if(argc > 1) {
			rc = cmdline(argc,(const char **) argv);
			if(rc) {
				mode = SERVICE_MODE_NONE;
			}
		}

		if(mode == SERVICE_MODE_FOREGROUND) {

			Logger::redirect(true);
			info() << "Running as application" << endl;

			try {
				init();
				rc = run();
				deinit();
			} catch(const std::exception &e) {
				error() << e.what() << endl;
				rc = -1;
			}
		}

		if(mode == SERVICE_MODE_DEFAULT || mode == SERVICE_MODE_DAEMON) {

			Logger::redirect(false);
			info() << "Starting service dispatcher" << endl;

			// Run as service by default.
			static SERVICE_TABLE_ENTRY DispatchTable[] = {
				{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) Service::Controller::dispatcher },
				{ NULL, NULL }
			};

			DispatchTable[0].lpServiceName = TEXT( (char *) appname.c_str());

			if(!StartServiceCtrlDispatcher( DispatchTable )) {
				error() << "Failed to start service dispatcher: " << endl << Win32::Exception::format(GetLastError()) << endl;
				return -1;
			}

		}

		return rc;

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

