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

	 SystemService::SystemService(const char *n) : name(n) {
	 }

	 SystemService::~SystemService() {
	 }

	 void SystemService::init() {
	 }

	 void SystemService::run() {
		MainLoop::getInstance().run();
	 }

	 void SystemService::deinit() {
	 }

	 void SystemService::start() {

		try {

			cout << name << "\tInitializing service" << endl;
			init();

			cout << name << "\tRunning service" << endl;
			run();

			cout << name << "\tDeinitializing service" << endl;
			deinit();

		} catch(const exception &e) {
			cerr << name << "\t" << e.what() << endl;
		} catch(...) {
			cerr << name << "\tUnexpected error running system service" << endl;
		}

	 }

	 void SystemService::stop() {
		MainLoop::getInstance().quit();
	 }

 }

