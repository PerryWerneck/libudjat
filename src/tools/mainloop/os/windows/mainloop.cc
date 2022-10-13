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
 #include "private.h"
 #include <private/mainloop.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>
 #include <private/event.h>

 #include <iostream>

 namespace Udjat {

	static ATOM	winClass = 0;

	MainLoop::MainLoop() {

		if(!winClass) {

			WNDCLASSEX wc;

			memset(&wc,0,sizeof(wc));

			wc.cbSize 			= sizeof(wc);
			wc.style  			= CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc  	= hwndProc;
			wc.hInstance  		= GetModuleHandle(NULL);
			wc.lpszClassName  	= PACKAGE_NAME;
			wc.cbWndExtra		= sizeof(LONG_PTR);

			winClass = RegisterClassEx(&wc);

			if(!winClass) {
				// throw Win32::Exception("Error calling RegisterClass");
				throw runtime_error("Failed: Can't register window class");
			}

		}

		// Create object window.
		hwnd = CreateWindow(
			PACKAGE_NAME,
			"MainLoop",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			0,
			CW_USEDEFAULT,
			0,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL
		);

		if(!hwnd) {
			throw runtime_error("Can't create mainloop window");
		}

		SetWindowLongPtr(hwnd, 0, (LONG_PTR) this);

		SetTimer(hwnd,IDT_CHECK_TIMERS,100,(TIMERPROC) NULL);

		debug("Main Object window was created");

	}

	MainLoop::~MainLoop() {

		KillTimer(hwnd, IDT_CHECK_TIMERS);
		DestroyWindow(hwnd);
		hwnd = 0;

		debug("Main Object window was destroyed");

	}

	void MainLoop::wakeup() noexcept {
		if(!hwnd) {
			Logger::String("Unexpected call to wakeup() without an active window").write(Logger::Trace,"MainLoop");
		} else if(!PostMessage(hwnd,WM_WAKE_UP,0,0)) {
			cerr << "MainLoop\tError posting wake up message to " << hex << hwnd << dec << " : " << Win32::Exception::format() << endl;
		}
	}

 	BOOL MainLoop::post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept {
 		return PostMessage(hwnd,uMsg,wParam,lParam);
	}

	LRESULT WINAPI MainLoop::hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		MainLoop & controller = *((MainLoop *) GetWindowLongPtr(hWnd,0));

		debug("------------------------------------- uMsg=",uMsg);

		try {

			switch(uMsg) {
			case WM_QUIT:
				Logger::String("WM_QUIT").write(Logger::Trace,"MainLoop");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_DESTROY:
				Logger::String("WM_DESTROY").write(Logger::Trace,"MainLoop");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_STOP:
				Logger::String("Stoppping services in response to a 'WM_STOP' message").write(Logger::Trace,"MainLoop");
				for(auto service : controller.services) {
					cout << "services\tStopping '" << service->name() << "'" << endl;
					try {
						service->stop();
					} catch(const std::exception &e) {
						cerr << "services\tError stopping service: " << e.what() << endl;
					} catch(...) {
						cerr << "services\tUnexpected error stopping service" << endl;
					}
				}
				break;

			case WM_WAKE_UP:

				// Check if the mainloop still enabled.
				debug("WM_WAKE_UP");

				if(controller.enabled) {

					// Controller is enabled, update timers.
					SendMessage(controller.hwnd,WM_CHECK_TIMERS,0,0);

				} else {

					// Controller is not enabled, stop.

					cout << "MainLoop\tMain loop was disabled" << endl;
					SendMessage(controller.hwnd,WM_STOP,0,0);

					if(!PostMessage(controller.hwnd,WM_QUIT,0,0)) {
						cerr << "MainLoop\tError posting WM_QUIT message to " << hex << controller.hwnd << dec << " : " << Win32::Exception::format() << endl;
					}
				}
				return 0;

			case WM_CHECK_TIMERS:
				{
					debug("WM_CHECK_TIMERS");

					UINT interval = controller.timers.run();

					debug("interval=",interval);

					if(interval) {
						SetTimer(controller.hwnd,IDT_CHECK_TIMERS,interval,(TIMERPROC) NULL);
					}

				}

				break;

			case WM_TIMER:
				if(wParam == IDT_CHECK_TIMERS) {
					PostMessage(controller.hwnd,WM_CHECK_TIMERS,0,0);
				}
				break;

			case WM_CONSOLE_HANDLER:
				debug("WM_CONSOLE_HANDLER");
				Event::ConsoleHandler((DWORD) wParam);
				return 0;

			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			}

		} catch(const exception &e) {

			cerr << "mainloop\t" << e.what() << endl;

		} catch(...) {

			cerr << "mainloop\tUnexpected error processing windows message" << endl;

		}

		return 0;

	}

 }
