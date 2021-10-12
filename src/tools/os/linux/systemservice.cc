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
 #include <udjat/tools/systemservice.h>
 #include <iostream>
 #include <system_error>
 #include <udjat/tools/mainloop.h>

 using namespace std;

 namespace Udjat {

	 SystemService::SystemService() {
	 }

	 SystemService::~SystemService() {
	 }

	 void SystemService::mainloop() {
		MainLoop::getInstance().run();
	 }

	 void SystemService::setup() {
	 }

	 int SystemService::run() {

		try {

			mainloop();

		} catch(const system_error &e) {
			cerr << e.what() << endl;
			return e.code().value();
		} catch(const exception &e) {
			cerr << e.what() << endl;
			return -1;
		} catch(...) {
			cerr << "Unexpected error running system service" << endl;
			return -1;
		}

		return 0;
	 }

 }

