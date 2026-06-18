/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <string>
 #include <udjat/tools/argumentparser.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <private/logger.h>
 #include <fstream> // std::filebuf

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifndef _WIN32
	#include <sys/resource.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	ArgumentParser & ArgumentParser::add_logger_group() {

		append(
			_( "Logger options"),
				ArgumentParser::Argument{
					'v', "verbose", _( "Enable console output" ),
					[](const char *argument, char mode) {
						return false;
					}
				},
				ArgumentParser::Argument{
					'q', "quiet", _( "Disable console output" ),
					[](const char *, char) {
						Logger::Controller::getInstance().console(false);
						return false;
					}
				},
				ArgumentParser::Argument{
					'l', "logfile", _( "Enable log to file" ),
					[](const char *argument, char) {
						if(!(argument && *argument)) {
							throw runtime_error(_( "Log to file requires a filename" ));
						}
						Logger::Controller::getInstance().file(argument);
						return false;
					}
				},
				ArgumentParser::Argument{
					'L', "loglevel", _( "Set the log verbosity level" ),
					[](const char *argument, char) {
						if(argument && *argument) {
							Logger::Controller::getInstance().verbosity(argument);
						}
						return false;
					}
				}
		);

#ifndef _WIN32
		add_application_argument(
			ArgumentParser::Argument{
				'C', "coredump", _( "Enable coredump" ),
				[](const char *pattern, char) {

					// Reference script:
					//
					// ulimit -c unlimited
					// install -m 1777 -d /var/local/dumps
					// echo "/var/local/dumps/core.%e.%p"> /proc/sys/kernel/core_pattern
					// rcapparmor stop
					// sysctl -w kernel.suid_dumpable=2
					//
					struct rlimit core_limits;
					memset(&core_limits,0,sizeof(core_limits));

					core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
					setrlimit(RLIMIT_CORE, &core_limits);

					if(pattern && *pattern) {
						// Set corepattern
						std::filebuf fb;
						fb.open("/proc/sys/kernel/core_pattern",std::ios::out);
						std::ostream os(&fb);
						os << pattern << "\n";
						fb.close();
					}

					return false;
				}
			}
		);
#endif // !_WIN32

		return *this;
	}

 }

