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

		SetTimer(hwnd,IDT_CHECK_TIMERS,100,(TIMERPROC) NULL);

		debug("Main Object window was created");

		/*
		// Terminate on Ctrl-C
		Udjat::Event::ConsoleHandler(this,CTRL_C_EVENT,[this](){
			Logger::String("Terminating by console request").write((Logger::Level) (Logger::Trace+1),"win32");
			exit(-1);
			return true;
		});

		//
		// Apply console handlers.
		//
		static const struct Events {
			DWORD id;
			bool def;
			const char * key;
			const char * message;
		} events[] = {
			{ CTRL_C_EVENT,			true,	"terminate-on-ctrl-c",		"Ctrl-C to interrupt"	},
			{ CTRL_BREAK_EVENT,		false,	"terminate-on-ctrl-break",	""						},
			{ CTRL_SHUTDOWN_EVENT,	false,	"terminate-on-shutdown",	""						},
		};

		for(size_t ix = 0; ix < (sizeof(events)/sizeof(events[0]));ix++) {

			if(Config::Value<bool>("win32",events[ix].key,events[ix].def)) {

				Udjat::Event::ConsoleHandler(this,events[ix].id,[this](){
					Logger::String("Terminating by console request").write((Logger::Level) (Logger::Trace+1),"win32");
					quit();
					return true;
				});

				if(events[ix].message[0]) {
					cout << "mainloop\t" << events[ix].message << endl;
				}
			}

		}
		*/

	}

	MainLoop & MainLoop::getInstance() {
		static MainLoop instance;
		return instance;
	}

	MainLoop::~MainLoop() {

		Udjat::Event::remove(this);

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

		// debug("uMsg=",uMsg);

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
				Logger::String("WM_POWERBROADCAST").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_DEVICECHANGE:
				Logger::String("WM_DEVICECHANGE").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_DESTROY:
				Logger::String("WM_DESTROY").write(Logger::Trace,"win32");
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_QUIT:
				Logger::String("WM_QUIT").write(Logger::Trace,"win32");
				controller.enabled = false; // Just in case.
				controller.stop();
				ThreadPool::getInstance().stop();
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_START:
				Logger::String("Starting services in response to a 'WM_START' message").write(Logger::Trace,"win32");
				ThreadPool::getInstance();
				controller.start();
				break;

			case WM_STOP:
				if(controller.enabled) {
					Logger::String("WM_STOP: Disabling mainloop").write(Logger::Trace,"win32");
					controller.enabled = false;
					if(!PostMessage(controller.hwnd,WM_QUIT,0,0)) {
						cerr << "win32\tError posting WM_QUIT message to " << hex << controller.hwnd << dec << " : " << Win32::Exception::format() << endl;
					}
				} else {
					Logger::String("WM_STOP: Already disabled").write(Logger::Trace,"win32");
				}
				break;

			case WM_WAKE_UP:
				if(controller.hwnd) {
					SendMessage(controller.hwnd,WM_CHECK_TIMERS,0,0);
				}
				break;

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

			default:
#ifdef DEBUG
				Logger::String("uMsg=",uMsg).write(Logger::Trace,"win32");
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
