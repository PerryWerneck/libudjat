/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/module.h>
 #include <private/updater.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	int Application::run(int argc, char **argv, const char *definitions) {

		int rc = -1;

		// Check for command line arguments.

		// Initialize
		try {
			setup(definitions,true);
		} catch(const std::exception &e) {
			cerr << e.what() << endl;
			return -1;
		}

		// Run
		MainLoop::getInstance().run();

		// Deinitialize
		try {
			ThreadPool::getInstance().wait();
			Module::unload();
		} catch(const std::exception &e) {
			cerr << e.what() << endl;
			return -1;
		}

		return rc;

	}

 }

