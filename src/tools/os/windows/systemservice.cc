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
 #include <udjat.h>
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
 #include <private/event.h>

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
					cerr << "winsrvc\tService handler called with cmd " << CtrlCmd << " without an active service" << endl;
					controller.set(controller.status.dwCurrentState, 0);
					return;
				}

				try {

					switch (CtrlCmd) {
					case SERVICE_CONTROL_SESSIONCHANGE:
						cout << "win32\tSERVICE_CONTROL_SESSIONCHANGE" << endl;
						break;

					case SERVICE_CONTROL_POWEREVENT:
						cout << "win32\tSERVICE_CONTROL_POWEREVENT" << endl;
						break;

#ifdef SERVICE_CONTROL_PRESHUTDOWN
					case SERVICE_CONTROL_PRESHUTDOWN:
						clog << "win32\tSERVICE_CONTROL_PRESHUTDOWN" << endl;
						controller.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","pre-shutdown-timer",30000));
						SystemService::getInstance()->stop();
						break;
#endif // SERVICE_CONTROL_PRESHUTDOWN

					case SERVICE_CONTROL_SHUTDOWN:
						clog << "win32\tSERVICE_CONTROL_SHUTDOWN" << endl;
						controller.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","shutdown-timer",30000));
						SystemService::getInstance()->stop();
						break;

					case SERVICE_CONTROL_STOP:
						cout << "win32\tSERVICE_CONTROL_STOP" << endl;
						controller.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","stop-timer",30000));
						SystemService::getInstance()->stop();
						break;

					case SERVICE_CONTROL_INTERROGATE:
						cout << "win32\tSERVICE_CONTROL_INTERROGATE" << endl;
						controller.set(controller.status.dwCurrentState, 0);
						break;

					default:
						clog << "win32\tUnexpected win32 service control code: " << ((int) CtrlCmd) << endl;
						controller.set(controller.status.dwCurrentState, 0);
					}

				} catch(const std::exception &e) {

					cerr << "win32\tError '" << e.what() << "' handling service request" << endl;
					// FIXME: Report failure.
					controller.set(controller.status.dwCurrentState, 0);

				} catch(...) {

					cerr << "win32\tUnexpected error handling service request" << endl;
					// FIXME: Report failure.
					controller.set(controller.status.dwCurrentState, 0);

				}

			}

			static void dispatcher() {

				Logger::redirect(false,true);
				cout << "win32\tRegistering " << PACKAGE_NAME << " service dispatcher" << endl;

				Controller &controller = getInstance();

				Udjat::Event::ConsoleHandler(&controller,CTRL_SHUTDOWN_EVENT,[](){
					auto service{SystemService::getInstance()};
					if(service) {
						service->trace() << "CTRL_SHUTDOWN_EVENT" << endl;
						getInstance().set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","stop-timer",30000));
						service->stop();
					}
					return false;
				});

				// Inicia como serviço
				controller.hStatus = RegisterServiceCtrlHandler(TEXT(Application::Name::getInstance().c_str()),handler);
				if(!controller.hStatus) {
					cerr << "win32\t" << Win32::Exception::format("RegisterServiceCtrlHandler failed") << endl;
					return;
				}

				auto service = SystemService::getInstance();

				if(service) {

					service->info() << "Running as windows service" << endl;

					try {

						controller.set(SERVICE_START_PENDING, Config::Value<unsigned int>("service","start-timer",30000));
						service->init();

						controller.set(SERVICE_RUNNING, 0);
						service->run();

					} catch(const std::exception &e) {

						service->error() << "Error '" << e.what() << "' running service" << endl;

					} catch(...) {

						service->error() << "Unexpected error running service" << endl;

					}

					controller.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","stop-timer",30000));
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

		if(message && *message) {

			try {

				Win32::Registry registry("service",true);

				registry.set("status",message);
				registry.set("status_time",TimeStamp().to_string().c_str());

				Logger::write((Logger::Level) (Logger::Trace+1),name().c_str(),message);

			} catch(const std::exception &e) {

				error() << "Error '" << e.what() << "' setting service state" << endl;

			}

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

				debug("RegCreateKeyEx(HKLM/SOFTWARE) rc was ",rc);
				throw Win32::Exception("Cant open application registry",rc);

			}

		}

		debug("Registry cleanup complete");
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

				debug("Auto close timer set to ",seconds);

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
				File::Path{instpath}.remove(true);

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

	static void terminate_by_console_event(const char *msg) noexcept {
		Logger::String(msg).write((Logger::Level) (Logger::Trace+1),"win32");
		MainLoop &mainloop{MainLoop::getInstance()};
		if(mainloop) {
			mainloop.quit();
		}
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

		debug("Service mode=",mode);

		if(mode == SERVICE_MODE_FOREGROUND) {

			Logger::redirect(true,true);
			info() << "Running as application with " << PACKAGE_NAME << " revision " << revision() << endl;

			Udjat::Event::ConsoleHandler(this,CTRL_C_EVENT,[](){
				terminate_by_console_event("Terminating by ctrl-c event");
				return true;
			});

			Udjat::Event::ConsoleHandler(this,CTRL_CLOSE_EVENT,[](){
				terminate_by_console_event("Terminating by close event");
				return true;
			});

			Udjat::Event::ConsoleHandler(this,CTRL_SHUTDOWN_EVENT,[](){
				terminate_by_console_event("Terminating by shutdown event");
				return true;
			});

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

			debug("Running as a service");

			Logger::redirect(false,true);
			info() << "Starting service dispatcher with " << PACKAGE_NAME << " revision " << revision() << endl;

			// Run as service by default.
			static SERVICE_TABLE_ENTRY DispatchTable[] = {
				{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) Service::Controller::dispatcher },
				{ NULL, NULL }
			};

			DispatchTable[0].lpServiceName = TEXT( (char *) appname.c_str());

			if(!StartServiceCtrlDispatcher( DispatchTable )) {
				error() << "Failed to start service dispatcher: " << Win32::Exception::format(GetLastError()) << endl;
				return -1;
			}

		}

		debug("Service ends with rc=",rc);
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

