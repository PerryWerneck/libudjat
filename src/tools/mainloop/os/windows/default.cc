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
 #include "private.h"
 #include <csignal>
 #include <cstring>
 #include <iostream>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/event.h>

 namespace Udjat {

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

	Win32::MainLoop & Win32::MainLoop::getInstance() {
		static Win32::MainLoop instance;
		return instance;
	}

 	MainLoop & MainLoop::getInstance() {
 		return Win32::MainLoop::getInstance();
	}

	Win32::MainLoop::MainLoop() : Udjat::MainLoop() {

		for(size_t ix = 0; ix < (sizeof(events)/sizeof(events[0]));ix++) {

			if(Config::Value<bool>("win32",events[ix].key,events[ix].def)) {

				Udjat::Event::ConsoleHandler(this,events[ix].id,[this](){
					clog << "mainloop\tTerminating by console request" << endl;
					enabled = false;
					wakeup();
					return true;
				});

				cout << "mainloop\t" << events[ix].message << endl;
			}

		}

	}

	Win32::MainLoop::~MainLoop() {
		Udjat::Event::remove(this);
	}

 }
