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

/**
 * @file
 *
 * @brief Implement windows main loop.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <udjat-internals.h>
 #include <iostream>

 using namespace std;

 void Udjat::MainLoop::run() {

	enabled = true;
	cout << "Main loop starts" << endl;

	MSG msg;
	memset(&msg,0,sizeof(msg));

	int rc = -1;
	while( (rc = GetMessage(&msg, NULL, 0, 0)) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(rc == 0) {
		cout << "Main loop has ended normally" << endl;
		return;
	}

	cerr << "Main loop has ended with windows error " << GetLastError() << endl;

 }

