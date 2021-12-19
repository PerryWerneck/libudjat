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

 #define _GNU_SOURCE
 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/application.h>
 #include <errno.h>

 using namespace std;

 namespace Udjat {

	Application::Name::Name(bool with_path) : string(with_path ? program_invocation_name : program_invocation_short_name) {
	}

	Application::DataDir::DataDir() : string{STRINGIZE_VALUE_OF(DATADIR) "/"} {
		append(program_invocation_short_name);
		append("/");
	}

	Application::LibDir::LibDir() : string{STRINGIZE_VALUE_OF(LIBDIR) "/"} {
	}

	Application::LibDir::LibDir(const char *subdir) : LibDir() {
		append(program_invocation_short_name);
		append("-");
		append(subdir);
		append("/");
	}

	Application::SysConfigDir::SysConfigDir() : string{"/etc/"} {
	}

	Application::SysConfigDir::SysConfigDir(const char *subdir) : SysConfigDir() {
		append(program_invocation_short_name);
		append("/");
		append(subdir);
		append("/");
	}

 }
