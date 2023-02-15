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
 #include <udjat/tools/application.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/agent/abstract.h>
 #include <stdexcept>
 #include <udjat/tools/event.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/intl.h>
 #include <udjat/win32/service.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	void Win32::Service::Status::set(SERVICE_STATUS_HANDLE handle, DWORD state, DWORD wait) noexcept {

		if(state == dwCurrentState) {
			return;
		}

		static const struct {
			DWORD state;
			const char *msg;
		} st[] = {
			{ SERVICE_START_PENDING,	"Service is pending start" },
			{ SERVICE_STOP_PENDING,		"Service is pending stop"	},
			{ SERVICE_STOPPED,			"Service is stopped"		},
			{ SERVICE_RUNNING,			"Service is running"		},
		};

		if(Logger::enabled(Logger::Trace)) {
			for(size_t f = 1; f < (sizeof(st)/sizeof(st[0]));f++) {
				if(st[f].state == state) {
					Logger::String{st[f].msg}.trace("win32");
					break;
				}
			}
		}

		dwCurrentState = state;
		dwWaitHint = wait;

		if(!SetServiceStatus(handle, (SERVICE_STATUS *) this)) {
			Logger::String{"SetServiceStatus: ",Win32::Exception::format()}.error("win32");
		}

	}

	void WINAPI Win32::Service::Controller::handler( DWORD CtrlCmd ) {

		Controller &controller = getInstance();

		try {

			switch (CtrlCmd) {
			case SERVICE_CONTROL_SESSIONCHANGE:
				Logger::String{"SERVICE_CONTROL_SESSIONCHANGE"}.trace("win32");
				break;

			case SERVICE_CONTROL_POWEREVENT:
				Logger::String{"SERVICE_CONTROL_POWEREVENT"}.trace("win32");
				break;

#ifdef SERVICE_CONTROL_PRESHUTDOWN
			case SERVICE_CONTROL_PRESHUTDOWN:
				Logger::String{"SERVICE_CONTROL_PRESHUTDOWN"}.trace("win32");
				controller.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","pre-shutdown-timer",30000));
				MainLoop::getInstance().quit("SERVICE_CONTROL_PRESHUTDOWN");
				break;
#endif // SERVICE_CONTROL_PRESHUTDOWN

			case SERVICE_CONTROL_SHUTDOWN:
				Logger::String{"SERVICE_CONTROL_SHUTDOWN"}.trace("win32");
				controller.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","shutdown-timer",30000));
				MainLoop::getInstance().quit("SERVICE_CONTROL_PRESHUTDOWN");
				break;

			case SERVICE_CONTROL_STOP:
				Logger::String{"SERVICE_CONTROL_STOP"}.trace("win32");
				controller.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","stop-timer",30000));
				MainLoop::getInstance().quit("SERVICE_CONTROL_STOP");
				break;

			case SERVICE_CONTROL_INTERROGATE:
				Logger::String{"SERVICE_CONTROL_INTERROGATE"}.trace("win32");
				controller.set(controller.status.dwCurrentState, 0);
				break;

			default:
				Logger::String{"Unexpected win32 service control code: ",((int) CtrlCmd)}.warning("win32");
				controller.set(controller.status.dwCurrentState, 0);
			}

		} catch(const std::exception &e) {

			Logger::String{"Error '",e.what(),"' handling service request"}.error("win32");
			// FIXME: Report failure.
			controller.set(controller.status.dwCurrentState, 0);

		} catch(...) {

			Logger::String{"Unexpected error handling service request"}.error("win32");
			// FIXME: Report failure.
			controller.set(controller.status.dwCurrentState, 0);

		}

	}

	int SystemService::argument(char opt, const char *optstring) {

		switch(opt) {
		case 'h':
			cout	<< "  --start\t\tStart service" << endl
					<< "  --uninstall\t\tStop service" << endl;
			break;

		case 'f':
			mode = Foreground;
			return Application::argument(opt,optstring);

		default:
			return Application::argument(opt,optstring);

		}

		return 1;
	}

	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

		int rc = Application::init(definitions);


		return rc;
	}

	int SystemService::run(const char *definitions) {

		int rc = 0;

		switch(mode) {
		case Foreground:
			rc = Application::run(definitions);
			break;

		case Default:
		case Daemon:

			throw runtime_error("NOT IMPLEMENTED (YET)");

			break;
		}

		return rc;

	}


 }

