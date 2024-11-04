/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tests.h>
 #include <udjat/tools/xml.h>
 #include <iostream>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	int Testing::Application::run_tests(int argc, char **argv, const Udjat::ModuleInfo &info) {

		Logger::redirect();
		Logger::verbosity(9);
		Logger::console(true);

		auto rc = Testing::Application{info}.run(argc,argv,"./test.xml");

		Logger::String{"Application exits with rc=",rc}.info("test");
		return rc;

	}

	Testing::Application::Application(const Udjat::ModuleInfo &info) :RandomFactory{info} {

	}

	Testing::Application::Application(const Udjat::ModuleInfo &info, const std::function<void()> &initialize)
		: Application{info} {
		initialize();
	}

	int Testing::Application::install(const char *name) {
		ShortCut{}.desktop();
		return super::install(name);
	}

	int Testing::Application::uninstall() {
		ShortCut{}.remove();
		return super::uninstall();
	}

	void Testing::Application::root(std::shared_ptr<Abstract::Agent>) {
		debug("test-arg='",getProperty("test-arg","default"));
	}


 }

