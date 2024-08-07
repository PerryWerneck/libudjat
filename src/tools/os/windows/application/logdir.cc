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
 #include <udjat/win32/registry.h>
 #include <private/win32.h>

 namespace Udjat {

	Application::LogDir::LogDir(const char *subdir) {

		try {

			std::string registry{Win32::Registry{"paths"}.get("logs","")};

			if(!registry.empty()) {
				std::string::assign(registry);
				mkdir();
			}

		} catch(...) {

			// Ignore errors (LogDir::getInstance() is called from Logger::write)

		}

		if(std::string::empty()) {

			// No registry, scan for default path.
			std::string::assign(Win32::PathFactory(FOLDERID_ProgramData,"logs"));
			mkdir();

		}

		if(subdir && *subdir) {
			append(subdir);
			mkdir();
			append("\\");
		}

	}

 }

