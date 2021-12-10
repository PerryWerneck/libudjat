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

		SetTimer(hwnd,IDT_CHECK_TIMERS,1000,(TIMERPROC) NULL);

	}

	MainLoop::~MainLoop() {
	}

	void MainLoop::wakeup() noexcept {
		PostMessage(hwnd,WM_WAKE_UP,0,0);
	}

	LRESULT WINAPI MainLoop::hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		MainLoop & controller = *((MainLoop *) GetWindowLongPtr(hWnd,0));

		try {

			switch(uMsg) {
			case WM_WAKE_UP:

				// Check if the mainloop still enabled.
				if(!controller.enabled) {
					PostMessage(hwnd,WM_QUIT,0,0);
					return 0;
				}

				// Enqueue a timer update.
				PostMessage(hwnd,WM_CHECK_TIMERS,0);

				break;

			case WM_CHECK_TIMERS:
				SetTimer(hwnd,IDT_CHECK_TIMERS,max(Timers::run(),1000L),(TIMERPROC) NULL);
				break;

			case WM_TIMER:
				if(wParam == IDT_CHECK_TIMERS) {
					PostMessage(hwnd,WM_CHECK_TIMERS,0);
				}
				break;

			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			}

		} catch(const exception &e) {

			cerr << "mainloop\t" << e.what << endl;

		} catch(...) {

			cerr << "mainloop\tUnexpected error processing windows message" << endl;

		}

		return 0;

	}

 }
