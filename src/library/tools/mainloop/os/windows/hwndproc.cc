/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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

 //
 // References:
 //
 // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms742267(v=vs.85)
 //

 #define LOG_DOMAIN "win32"

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/win32/mainloop.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/threadpool.h>
 #include <private/service.h>
 #include <private/event.h>
 #include <iostream>
 #include <udjat/win32/exception.h>

 using namespace std;

 namespace Udjat {

	LRESULT WINAPI Win32::MainLoop::hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		MainLoop & controller = *((MainLoop *) GetWindowLongPtr(hWnd,0));

		try {

			switch(uMsg) {
			case WM_CREATE:
				Logger::String("WM_CREATE").trace();
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_DESTROY:
				Logger::String("WM_DESTROY").trace();
				if(controller.hwnd) {
					Logger::String{"Stopping by system request (WM_DESTROY)"}.warning();
					SendMessage(hWnd,WM_STOP,0,0);
				}
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_CLOSE:
				Logger::String("WM_CLOSE").trace();
				if(controller.hwnd) {
					Logger::String{"Stopping by system request (WM_CLOSE)"}.warning();
					SendMessage(hWnd,WM_STOP,0,0);
				}
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_QUERYENDSESSION:
				Logger::String("WM_QUERYENDSESSION").trace();
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_ENDSESSION:
				Logger::String("WM_ENDSESSION").trace();
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_POWERBROADCAST:
				switch(wParam) {
				case PBT_APMPOWERSTATUSCHANGE:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMPOWERSTATUSCHANGE: Power status has changed."
					).trace();
					break;

				case PBT_APMRESUMEAUTOMATIC:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMRESUMEAUTOMATIC: Operation is resuming automatically from a low-power state."
					).trace();
					break;

				case PBT_APMRESUMESUSPEND:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMRESUMESUSPEND: Operation is resuming from a low-power state."
					).trace();
					break;

				case PBT_APMSUSPEND:
					Logger::String(
						"WM_POWERBROADCAST.PBT_APMSUSPEND: System is suspending operation."
					).trace();
					break;

				case PBT_POWERSETTINGCHANGE:
					Logger::String(
						"WM_POWERBROADCAST.PBT_POWERSETTINGCHANGE: A power setting change event has been received."
					).trace();
					break;

				default:
					Logger::String(
						"WM_POWERBROADCAST.",((unsigned int) wParam),": Unexpected power event"
					).trace();
				}
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

//			case WM_DEVICECHANGE:
//				Logger::String("WM_DEVICECHANGE").trace();
//				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			case WM_START:
				Logger::String("WM_START: Initializing").trace();
				ThreadPool::getInstance();
				Service::Controller::getInstance().start();
				SetTimer(hWnd,IDT_CHECK_TIMERS,controller.uElapse = 100,(TIMERPROC) NULL);
				break;

			case WM_STOP_WITH_MESSAGE:
				Logger::String{(LPCTSTR)lParam}.write((Logger::Level) wParam,LOG_DOMAIN);
				PostMessage(controller.hwnd,WM_STOP,0,0);
				break;

			case WM_STOP:
				Logger::String("WM_STOP").trace();

				KillTimer(hWnd, IDT_CHECK_TIMERS);
				Event::clear();

				if(controller.hwnd) {
					Logger::String("WM_STOP: Terminating").trace();

					debug("Stopping services");
					Service::Controller::getInstance().stop();

					debug("Stoppping threadpool");
					ThreadPool::getInstance().stop();

					debug("Posting QUIT message");
					if(!PostMessage(controller.hwnd,WM_QUIT,0,0)) {
						Logger::error() << "win32\tError posting WM_QUIT message to " << hex << controller.hwnd << dec << " : " << Win32::Exception::format() << endl;
					}
					controller.hwnd = 0;
				} else {
					Logger::String("WM_STOP: Already disabled").warning();
				}
				break;

			case WM_TIMER:

				debug("WM_TIMER ", ((unsigned int) wParam));

				if(controller.hwnd) {

					if(wParam == IDT_CHECK_TIMERS) {

						if(!PostMessage(controller.hwnd,WM_CHECK_TIMERS,0,0)) {
							Logger::error() << "MainLoop\tError posting WM_CHECK_TIMERS up message to " << hex << controller.hwnd << dec << " : " << Win32::Exception::format() << endl;
						}

					} else {

						Logger::error() << "win32\tInvalid or unexpected timer ID " << ((unsigned int ) wParam) << endl;

					}

				} else {

					Logger::String("Got timer ", ((unsigned int) wParam), " on invalid window").write(Logger::Debug,"win32");

				}
				break;

			case WM_CHECK_TIMERS:
				{
					if(controller.timers.maxwait > 1000) {
						Logger::String{"Maxwait cant be greater than 1000, fixing"}.warning();
						controller.timers.maxwait = 1000;
					}

					unsigned long interval = controller.compute_poll_timeout();
					if(interval != controller.uElapse) {
						SetTimer(controller.hwnd, IDT_CHECK_TIMERS, controller.uElapse = interval, (TIMERPROC) NULL);
					}

				}
				break;

			case WM_ADD_TIMER:
				{
					lock_guard<mutex> lock(controller.guard);
					controller.timers.enabled.push_back((Timer *) lParam);
				}
				SendMessage(controller.hwnd,WM_CHECK_TIMERS,0,0);
				break;

			case WM_REMOVE_TIMER:
				{
					lock_guard<mutex> lock(controller.guard);
					controller.timers.enabled.remove((Timer *) lParam);
				}
				break;

			case WM_PROCESS_POSTED_MESSAGE:
				{
					Message *message = (Message *) lParam;
					message->execute();
					delete message;
				}
				break;	

				default:
#ifdef DEBUG
				Logger::trace() << "win32\tuMsg=" << hex << uMsg << dec << endl;
#endif // DEBUG
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			}

		} catch(const exception &e) {

			Logger::error() << "win32\t" << e.what() << endl;

		} catch(...) {

			Logger::error() << "win32\tUnexpected error processing windows message" << endl;

		}

		return 0;

	}

 }

