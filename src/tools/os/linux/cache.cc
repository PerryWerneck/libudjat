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
 #include <sys/types.h>
 #include <udjat/tools/logger.h>
 #include <pwd.h>

 using namespace std;

 namespace Udjat {

 	static std::string getCacheDir() {

		uid_t uid = getuid();

		if(uid == 0) {
			debug("Root user, using /var/cache");
			return "/var/cache/";
		}

		string cachedir;
		{
			long strbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
			char strbuf[strbuflen+1];
			struct passwd *pw = NULL;
			struct passwd pwbuf;

			if (getpwuid_r(uid, &pwbuf, strbuf, strbuflen, &pw) != 0 || pw == NULL) {
				throw system_error(errno,system_category(),"Error getting user homedir");
			}

			cachedir = pw->pw_dir;
		}

		cachedir.append("/.cache/");

		debug("Non root user, using ",cachedir);
		return cachedir;
 	}

	Application::CacheDir::CacheDir() : File::Path{getCacheDir().c_str()} {

		append(program_invocation_short_name);
		append("/");

		mkdir(0700);

		if(access(c_str(),W_OK) == 0)
			return;

		throw system_error(EPERM,system_category(),c_str());

	}

	Application::CacheDir::CacheDir(const char *subdir) : CacheDir() {

		append(subdir);
		append("/");

		mkdir(0700);

		if(access(c_str(),W_OK) == 0)
			return;

		throw system_error(EPERM,system_category(),c_str());

	}

 }
