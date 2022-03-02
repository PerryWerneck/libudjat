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

 #include "private.h"

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

#ifdef DEBUG
		cout << "Main Object window was created" << endl;
#endif // DEBUG

	}

	MainLoop::~MainLoop() {

		KillTimer(hwnd, IDT_CHECK_TIMERS);
		DestroyWindow(hwnd);

#ifdef DEBUG
		cout << "Main Object window was destroyed" << endl;
#endif // DEBUG
	}

	void MainLoop::wakeup() noexcept {
		if(!PostMessage(hwnd,WM_WAKE_UP,0,0)) {
			cerr << "win32\tWindows error " << GetLastError() << " while posting wake up message" << endl;
		}
	}

 	BOOL MainLoop::post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept {
		return PostMessage(hwnd,uMsg,wParam,lParam);
	}

	LRESULT WINAPI MainLoop::hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		MainLoop & controller = *((MainLoop *) GetWindowLongPtr(hWnd,0));

		try {

			switch(uMsg) {
			case WM_QUIT:
				cout << "MainLoop\tWM_QUIT" << endl;
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_DESTROY:
				cout << "MainLoop\tWM_DESTROY" << endl;
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_STOP:
				cout << "MainLoop\tWM_STOP - Stopping services" << endl;
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
#ifdef DEBUG
				cout <<  __FILE__ << "(" << __LINE__ << ")" << endl;
#endif //
				if(!controller.enabled) {
					cout << "MainLoop\tMain loop was disabled" << endl;
					SendMessage(controller.hwnd,WM_STOP,0,0);

					if(!PostMessage(controller.hwnd,WM_QUIT,0,0)) {
						cerr << "win32\tError " << GetLastError() << " posting WM_QUIT message" << endl;
					}
					return 0;
				}

				// Update timers.
				SendMessage(controller.hwnd,WM_CHECK_TIMERS,0,0);

				break;

			case WM_CHECK_TIMERS:
				{
					UINT interval = controller.timers.run();

//#ifdef DEBUG
//					cout <<  __FILE__ << "(" << __LINE__ << ") interval=" << interval << endl;
//#endif //
					if(interval) {
						SetTimer(controller.hwnd,IDT_CHECK_TIMERS,interval,(TIMERPROC) NULL);
					}

				}

				break;

			case WM_TIMER:
				if(wParam == IDT_CHECK_TIMERS) {
					SendMessage(controller.hwnd,WM_CHECK_TIMERS,0,0);
				}
				break;

			case WM_EVENT_ACTION:
				try {

					Win32::Event * event = Win32::Event::Controller::getInstance().find((HANDLE) lParam);
					if(event) {
						event->call(wParam != 0);
					}

				} catch(const std::exception &e) {
					cerr << "Win32\tError '" << e.what() << "' processing event handler" << endl;
				} catch(...) {
					cerr << "Win32\tUnexpected error processing event handler" << endl;
				}
				break;

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
