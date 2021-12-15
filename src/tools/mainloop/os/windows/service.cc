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

/**
 * @file
 *
 * @brief Implement windows main loop.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include "private.h"
 #include <udjat-internals.h>
 #include <iostream>

 using namespace std;

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

	static Status & getInstance();

 };

 Status & Status::getInstance() {
	static Status instance;
	return instance;
 }

 /// @brief Handle to the status information structure for the current service.
 static SERVICE_STATUS_HANDLE gStatusHandle;

 static void SetStatus( DWORD State, DWORD ExitCode, DWORD Wait ) {

	static const struct _state {
		DWORD State;
		const char *msg;
	} st[] = {
		{ SERVICE_START_PENDING,	"Service is pending start" },
		{ SERVICE_STOP_PENDING,		"Service is pending stop"	},
		{ SERVICE_STOPPED,			"Service is stopped"		},
		{ SERVICE_RUNNING,			"Service is running"		},
	};

	Status &status = Status::getInstance();

	if(State != status.dwCurrentState) {

		for(size_t f = 1; f < (sizeof(st)/sizeof(st[0]));f++) {
			if(st[f].State == State) {
				cout << "MainLoop\t" << st[f].msg << endl;
				break;
			}
		}
	}

	status.dwCurrentState	= State;
	status.dwWin32ExitCode	= ExitCode;
	status.dwWaitHint		= Wait;

	// gStatus.dwServiceType				= SERVICE_WIN32;
	// gStatus.dwServiceSpecificExitCode	= 0;
	// gStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	if(!SetServiceStatus(gStatusHandle, (SERVICE_STATUS *) &status)) {
		cerr << "MainLoop\tSystem error in SetServiceStatus()" << endl;
	}

 }

 ///
 /// @brief Manipulador para mensagens do controlador de serviços windows
 ///
 /// @param CtrlCmd Comando enviado pelo windows.
 ///
 static void WINAPI CtrlHandler( DWORD CtrlCmd ) {

	switch (CtrlCmd) {
	case SERVICE_CONTROL_SHUTDOWN:
			SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
			cout << "MainLoop\tSystem shutdown, stopping" << endl;
			Udjat::Win32::MainLoop::getInstance().stop();
			break;

	case SERVICE_CONTROL_STOP:
			SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
			cout << "MainLoop\tStopping by request" << endl;
			Udjat::Win32::MainLoop::getInstance().stop();
			break;

	case SERVICE_CONTROL_INTERROGATE:
			// Add here the necessary code to query the daemon
			SetStatus(Status::getInstance().dwCurrentState, NO_ERROR, 0);
			break;

	default:
			clog << "MainLoop\tUnexpected service control code: " << ((int) CtrlCmd) << endl;
			SetStatus(Status::getInstance().dwCurrentState, NO_ERROR, 0);
	}

 }

 static void entryWindowsService(void) {

	// Inicia como serviço
	gStatusHandle = RegisterServiceCtrlHandler(TEXT(PACKAGE_NAME), CtrlHandler);

	if(!gStatusHandle) {
		throw runtime_error("RegisterServiceCtrlHandler failed");
	}

	SetStatus(SERVICE_START_PENDING, NO_ERROR, 3000 );

	auto &controller = Udjat::Win32::MainLoop::getInstance();

	SetStatus(SERVICE_RUNNING, NO_ERROR, 0);

	try {

		controller.run();

	} catch(const std::exception &e) {

		cerr << "MainLoop\t" << e.what() << endl;

	} catch(...) {
		cerr << "MainLoop\tUnexpected error starting windows service" << endl;
	}

	SetStatus(SERVICE_STOPPED, NO_ERROR, 0);

 }

 void Udjat::MainLoop::start() {

	static const SERVICE_TABLE_ENTRY DispatchTable[] = {
		{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) entryWindowsService },
		{ NULL, NULL }
	};

	cout << "MainLoop\tStarting service dispatcher" << endl;

	if(!StartServiceCtrlDispatcher( DispatchTable )) {
		cerr << "MainLoop\tError in service dispatcher" << endl;
	}

 }

 void Udjat::MainLoop::stop() {
	Udjat::Win32::MainLoop::getInstance().enabled = false;
 }
