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
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/quark.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Application::Application(int &c, char **v) : argc{c}, argv{v} {

		Quark::init();

#ifdef DEBUG 
		Logger::console(true);
#endif
		Logger::setup(argc,argv);

#ifdef GETTEXT_PACKAGE
		set_gettext_package(GETTEXT_PACKAGE);
		setlocale( LC_ALL, "" );
#endif // GETTEXT_PACKAGE


	}

	Dialog::Status & Application::state(const Level, const char *message) noexcept {
		Logger::String{message}.write((Logger::Level) (Logger::Debug+1),Name().c_str());
		return *this;
	}

 }
