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
 #include <stdexcept>
 #include <udjat/win32/exception.h>
 #include <direct.h>

#ifdef HAVE_LIBINTL
	#include <libintl.h>
#endif // HAVE_LIBINTL

 using namespace std;

 namespace Udjat {

  	Application::Application() {

  		Quark::init();

		// Go to application path.
		_chdir(Path().c_str());

#ifdef GETTEXT_PACKAGE
		set_gettext_package(GETTEXT_PACKAGE);
		setlocale( LC_ALL, "" );
#endif // GETTEXT_PACKAGE

		// https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-/blob/master/docs/part-04.md
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);

		WSADATA WSAData;
		{
			int err = WSAStartup(MAKEWORD(2,2), &WSAData);
			if(err) {
				throw Win32::Exception(err);
			}
		}

		if(Config::Value<bool>("application","virtual-terminal-processing",true)) {
			// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if(hOut != INVALID_HANDLE_VALUE) {
				DWORD dwMode = 0;
				if(GetConsoleMode(hOut, &dwMode)) {
					dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
					SetConsoleMode(hOut,dwMode);
				}
			}
		}

	}

 }

