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

 #ifndef _GNU_SOURCE
	#define _GNU_SOURCE
 #endif // !_GNU_SOURCE

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module/abstract.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <system_error>
 #include <libintl.h>
 #include <iostream>
 #include <limits>
 #include <dirent.h>
 #include <signal.h>

 using namespace std;

 namespace Udjat {

	void Application::set_gettext_package(const char *gettext_package) {

#ifdef HAVE_LIBINTL

		bindtextdomain(gettext_package, STRINGIZE_VALUE_OF(LOCALEDIR));
		bind_textdomain_codeset(gettext_package, "UTF-8");
		debug("Locale set to ",STRINGIZE_VALUE_OF(LOCALEDIR),"/",gettext_package);

#endif // HAVE_LIBINTL

	}

	Application::Name::Name(bool with_path) : string{with_path ? program_invocation_name : program_invocation_short_name} {
	}

	static std::string PathFactory(const char *path, const char *subdir, bool required) {

		std::string response{path};

		if(path[strlen(path)-1] != '/') {
			response += '/';
		}

		debug("----------------------> [",response,"]");

		response.append(program_invocation_short_name);
		File::Path::mkdir(response.c_str(),required);
		response.append("/");

		if(subdir && *subdir) {
			response.append(subdir);
			File::Path::mkdir(response.c_str(),required);
			response.append("/");
		}

		return response;
	}

	Application::DataDir::DataDir(const char *subdir, bool required) : File::Path{PathFactory(STRINGIZE_VALUE_OF(DATADIR) "/",subdir,required)} {
	}

	static const char * get_tmpdir() {

		static const char *envvars[] = { "TEMP", "TMP", "TMPDIR" };

		for(const char *env : envvars) {

			const char *ptr = getenv(env);
			if(ptr) {
				return ptr;
			}

		}

		return "/tmp";
	}

	Application::TmpDir::TmpDir(const char *subdir) noexcept : File::Path{PathFactory(get_tmpdir(),subdir,true)} {
	}

	Application::LogDir::LogDir(const char *subdir) noexcept : File::Path{PathFactory("/var/log/",subdir,true)} {
	}

	Application::SystemDataDir::SystemDataDir(const char *subdir) : File::Path{PathFactory("/usr/share/",subdir,true)} {
	}

	Application::LibDir::LibDir(const char *subdir, bool required) : File::Path{PathFactory(STRINGIZE_VALUE_OF(LIBDIR) "/",subdir,required)} {
	}

	void Application::LibDir::reset(const char *application_name, const char *subdir) {
		assign(STRINGIZE_VALUE_OF(LIBDIR) "/");
		append(application_name);
		append("-");
		append(subdir);
		append("/");
	}

	Application::LibDir::operator bool() const noexcept {
		return (access(c_str(), R_OK) == 0);
	}

	Application::SysConfigDir::SysConfigDir(const char *subdir) : File::Path{PathFactory("/etc/",subdir,true)} {
	}

	size_t Application::signal(int signum) {

		string me{to_string(getpid())};

		char exename[PATH_MAX+1];
		memset(exename,0,PATH_MAX+1);

		if(readlink((string{"/proc/"}+me+"/exe").c_str(), exename, PATH_MAX) < 0) {
			throw system_error(errno,system_category());
		}

		debug("Me= '",me,"'");
		debug("ExeName= '",exename,"'");

		size_t count = 0;
		DIR *dir = opendir("/proc");
		if(!dir) {
			throw system_error(errno,system_category());
		}

		struct dirent *de;
		while((de = readdir(dir)) != NULL) {

			if(!de->d_name || de->d_name[0] == '.' || de->d_type != DT_DIR || (strcmp(de->d_name,me.c_str()) == 0))  {
				continue;
			}

			char procname[PATH_MAX+1];
			memset(procname,0,PATH_MAX+1);

			if(readlinkat(dirfd(dir), (string{de->d_name} + "/exe").c_str(), procname, PATH_MAX) < 0) {
				if(errno != ENOENT) {
					cerr << "/proc/" << de->d_name << "/exe  " << strerror(errno) << " (rc=" << errno << ")" << endl;
				}
				continue;
			}

			if(!strcmp(procname,exename)) {
				count++;
				if(signum) {
					cout << "Sending signal '" << signum << "' to process " << de->d_name << endl;
					kill((pid_t) stoi(de->d_name), signum);
				}
			}

		}
		closedir(dir);

		return count;
	}

 }
