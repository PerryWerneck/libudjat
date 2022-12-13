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
 #include <udjat/tools/configuration.h>

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
				throw Win32::Exception("Error calling RegisterClass");
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

		debug("Main Object window was created");

	}

	MainLoop & MainLoop::getInstance() {
		static MainLoop instance;
		return instance;
	}

	MainLoop::~MainLoop() {

		Udjat::Event::remove(this);

		DestroyWindow(hwnd);
		hwnd = 0;

		debug("Main Object window was destroyed");

	}

	void MainLoop::wakeup() noexcept {
		if(!hwnd) {
			Logger::String("Unexpected call to wakeup() without an active window").write(Logger::Trace,"MainLoop");
		} else if(!PostMessage(hwnd,WM_CHECK_TIMERS,0,0)) {
			cerr << "MainLoop\tError posting wake up message to " << hex << hwnd << dec << " : " << Win32::Exception::format() << endl;
		}
#ifdef DEBUG
		else {
			debug("WAKE-UP");
		}
#endif // DEBUG
	}

 	BOOL MainLoop::post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept {
 		return PostMessage(hwnd,uMsg,wParam,lParam);
	}

	LRESULT WINAPI MainLoop::hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		MainLoop & controller = *((MainLoop *) GetWindowLongPtr(hWnd,0));

		try {

			switch(uMsg) {
			case WM_CREATE:
				Logger::String("WM_CREATE").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_CLOSE:
				Logger::String("WM_CLOSE").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_QUERYENDSESSION:
				Logger::String("WM_QUERYENDSESSION").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_ENDSESSION:
				Logger::String("WM_ENDSESSION").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_POWERBROADCAST:
				switch(wParam) {
				case PBT_APMPOWERSTATUSCHANGE:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMPOWERSTATUSCHANGE: Power status has changed."
					).write(Logger::Trace,"win32");
					break;

				case PBT_APMRESUMEAUTOMATIC:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMRESUMEAUTOMATIC: Operation is resuming automatically from a low-power state."
					).write(Logger::Trace,"win32");
					break;

				case PBT_APMRESUMESUSPEND:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMRESUMESUSPEND: Operation is resuming from a low-power state."
					).write(Logger::Trace,"win32");
					break;

				case PBT_APMSUSPEND:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMSUSPEND: System is suspending operation."
					).write(Logger::Trace,"win32");
					break;

				case PBT_POWERSETTINGCHANGE:
					Logger::String(
						"WM_POWERBROADCAST.PBT_POWERSETTINGCHANGE: A power setting change event has been received."
					).write(Logger::Trace,"win32");
					break;

				default:
					Logger::String(
						"WM_POWERBROADCAST.",((unsigned int) wParam),": Unexpected power event"
					).write(Logger::Trace,"win32");
				}
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

//			case WM_DEVICECHANGE:
//				Logger::String("WM_DEVICECHANGE").write(Logger::Trace,"win32");
//				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_DESTROY:
				Logger::String("WM_DESTROY").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_START:
				Logger::String("WM_START: Initializing").write(Logger::Trace,"win32");
				ThreadPool::getInstance();
				controller.start();
				SetTimer(hWnd,IDT_CHECK_TIMERS,controller.uElapse = 100,(TIMERPROC) NULL);
				break;

			case WM_STOP:

				KillTimer(hWnd, IDT_CHECK_TIMERS);

				if(controller.enabled) {
					Logger::String("WM_STOP: Terminating").write(Logger::Trace,"win32");
					controller.enabled = false; // Just in case.
					controller.stop();
					ThreadPool::getInstance().stop();
					if(!PostMessage(controller.hwnd,WM_QUIT,0,0)) {
						cerr << "win32\tError posting WM_QUIT message to " << hex << controller.hwnd << dec << " : " << Win32::Exception::format() << endl;
					}
				} else {
					Logger::String("WM_STOP: Already disabled").write(Logger::Trace,"win32");
				}
				break;

			case WM_TIMER:

				Logger::String("WM_TIMER ", ((unsigned int) wParam)).write(Logger::Debug,"win32");

				if(controller.hwnd) {

					if(wParam == IDT_CHECK_TIMERS) {

						if(!PostMessage(controller.hwnd,WM_CHECK_TIMERS,0,0)) {
							cerr << "MainLoop\tError posting WM_CHECK_TIMERS up message to " << hex << controller.hwnd << dec << " : " << Win32::Exception::format() << endl;
						}

					} else {

						cerr << "win32\tInvalid or unexpected timer ID " << ((unsigned int ) wParam) << endl;

					}

				} else {

					Logger::String("Got timer ", ((unsigned int) wParam), " on invalid window").write(Logger::Debug,"win32");

				}
				break;

			case WM_CHECK_TIMERS:
				{
					UINT interval = controller.timers.run();
					if(!interval) {
						interval = 1000;
					}

					if(interval != controller.uElapse) {
						Logger::String("Reseting timer to ", interval).write(Logger::Debug,"win32");
						SetTimer(controller.hwnd, IDT_CHECK_TIMERS, controller.uElapse = interval, (TIMERPROC) NULL);
					} else {
						Logger::String("Keeping timer set to ", interval).write(Logger::Debug,"win32");
					}

				}
				break;

			default:
#ifdef DEBUG
				Logger::trace() << "win32\tuMsg=" << hex << uMsg << dec << endl;
#endif // DEBUG
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			}

		} catch(const exception &e) {

			cerr << "win32\t" << e.what() << endl;

		} catch(...) {

			cerr << "win32\tUnexpected error processing windows message" << endl;

		}

		return 0;

	}

 }
