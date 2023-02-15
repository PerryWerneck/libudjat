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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/win32/service.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/logger.h>
 #include <private/event.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 #include <stdexcept>

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

	void SystemService::dispatcher() {

		Logger::console(false);
		Logger::String{"Registering Service dispatcher"}.trace("win32");

		SystemService &service = getInstance();

		Udjat::Event::ConsoleHandler(&service,CTRL_SHUTDOWN_EVENT,[](){
			try {
				getInstance().set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","stop-timer",30000));
				MainLoop::getInstance().quit("CTRL_SHUTDOWN_EVENT");
			} catch(const std::exception &e) {
				Logger::String{"Error processing shutdown event: ",e.what()}.error("win32");
			}
			return false;
		});

		// Inicia como serviço
		service.hStatus = RegisterServiceCtrlHandler(TEXT(Application::Name::getInstance().c_str()),handler);
		if(!service.hStatus) {
			Logger::String{Win32::Exception::format("RegisterServiceCtrlHandler failed").c_str()}.error("win32");
			return;
		}

		try {

			service.set(SERVICE_START_PENDING, Config::Value<unsigned int>("service","start-timer",30000));
			if(!service.init(service.definitions)) {
				throw runtime_error("Error initializing service");
			}

			service.set(SERVICE_RUNNING, 0);
			MainLoop::getInstance().run();

			service.set(SERVICE_STOP_PENDING, Config::Value<unsigned int>("service","stop-timer",30000));
			service.deinit(service.definitions);

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error("win32");

		}

		service.set(SERVICE_STOPPED, 0);

	}

	void WINAPI SystemService::handler( DWORD CtrlCmd ) {

		SystemService &controller = getInstance();

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
				controller.set(controller.service_status.dwCurrentState, 0);
				break;

			default:
				Logger::String{"Unexpected win32 service control code: ",((int) CtrlCmd)}.warning("win32");
				controller.set(controller.service_status.dwCurrentState, 0);
			}

		} catch(const std::exception &e) {

			Logger::String{"Error '",e.what(),"' handling service request"}.error("win32");
			// FIXME: Report failure.
			controller.set(controller.service_status.dwCurrentState, 0);

		} catch(...) {

			Logger::String{"Unexpected error handling service request"}.error("win32");
			// FIXME: Report failure.
			controller.set(controller.service_status.dwCurrentState, 0);

		}

	}

 }
