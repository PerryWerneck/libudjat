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
 #include <udjat/tools/application.h>
 #include <fcntl.h>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	Application::CacheDir::CacheDir() : File::Path{"/var/cache/"} {

		append(program_invocation_short_name);
		append("/");

		if(::mkdir(c_str(), 0700) == 0)
			return;

		if(access(c_str(),W_OK) == 0)
			return;

		const char *homedir = getenv("HOME");
		if(!homedir)
			homedir = "~";

		assign(homedir);
		append("/.cache/");

		append(program_invocation_short_name);
		append("/");

		if(::mkdir(c_str(), 0700) == 0)
			return;

		if(access(c_str(),W_OK) == 0)
			return;

		throw system_error(EPERM,system_category(),c_str());

	}

	Application::CacheDir::CacheDir(const char *subdir) : CacheDir() {

		append(subdir);
		append("/");

		if(::mkdir(c_str(), 0700) == 0)
			return;

		if(access(c_str(),W_OK) == 0)
			return;

		throw system_error(EPERM,system_category(),c_str());

	}

 }
