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

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/win32/mainloop.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Win32::MainLoop::MainLoop() : Udjat::MainLoop{MainLoop::WinMsg} {

		WNDCLASSEX wc;

		memset(&wc,0,sizeof(wc));

		wc.cbSize 			= sizeof(wc);
		wc.style  			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc  	= hwndProc;
		wc.hInstance  		= GetModuleHandle(NULL);
		wc.lpszClassName  	= PACKAGE_NAME;
		wc.cbWndExtra		= sizeof(LONG_PTR);

		identifier = RegisterClassEx(&wc);

		if(!identifier) {
			throw Win32::Exception("Error calling RegisterClass");
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

	}

	Win32::MainLoop::~MainLoop() {

		DestroyWindow(hwnd);
		UnregisterClass(PACKAGE_NAME,GetModuleHandle(NULL));

	}

	void Win32::MainLoop::wakeup() noexcept {
		if(!hwnd) {
			Logger::String("Unexpected call to wakeup() without an active window").write(Logger::Trace,"MainLoop");
		} else if(!PostMessage(hwnd,WM_CHECK_TIMERS,0,0)) {
			cerr << "MainLoop\tError posting wake up message to " << hex << hwnd << dec << " : " << Win32::Exception::format() << endl;
		}
		debug("WAKE-UP");
	}

 	BOOL Win32::MainLoop::post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept {
 		return PostMessage(hwnd,uMsg,wParam,lParam);
	}

	bool Win32::MainLoop::enabled(const Timer *timer) const noexcept {
		lock_guard<mutex> lock(guard);
		for(Timer *tm : timers.enabled) {
			if(timer == tm) {
				return true;
			}
		}
		return false;
	}

	bool Win32::MainLoop::active() const noexcept {
		return hwnd != 0;
	}

	void Win32::MainLoop::post(MainLoop::Message *message) noexcept {
		if(!PostMessage(hwnd,WM_PROCESS_POSTED_MESSAGE,0,(LPARAM) message)) {
			delete message;
			Logger::String{"Error posting message"}.error();
		}
	}

	bool Win32::MainLoop::enabled(const Handler *handler) const noexcept {
		lock_guard<mutex> lock(guard);
		for(Handler *hdl : handlers) {
			if(handler == hdl) {
				return true;
			}
		}
		return false;
	}

	void Win32::MainLoop::push_back(MainLoop::Timer *timer) {
		post(WM_ADD_TIMER,0,(LPARAM) timer);
	}

	void Win32::MainLoop::remove(MainLoop::Timer *timer) {
		post(WM_REMOVE_TIMER,0,(LPARAM) timer);
	}

	void Win32::MainLoop::push_back(MainLoop::Handler *handler) {
		lock_guard<mutex> lock(guard);
		handlers.push_back(handler);
	}

	void Win32::MainLoop::remove(MainLoop::Handler *handler) {
		lock_guard<mutex> lock(guard);
		handlers.remove(handler);
	}

	bool Win32::MainLoop::for_each(const std::function<bool(Timer &timer)> &func) {
		lock_guard<mutex> lock(guard);
		for(auto timer : timers.enabled) {
			if(func(*timer)) {
				return true;
			}
		}
		return false;
	}

	void Win32::MainLoop::run(const std::function<void()> &method) {

		// TODO: Implement a way to run tasks in the main loop thread.
				
		throw system_error(ENOTSUP,system_category(),"Cant run GUI tasks in Win32 mainloop");

	}

 }

 