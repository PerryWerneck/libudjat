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

						auto &controller = Logger::Controller::getInstance();
						if(argument && *argument) {

							controller.console(true);
							controller.verbosity(argument);
							return ArgumentParser::Handled;

						} else {
							switch(mode) {
								case 'S':
								case 'L':
								case '0':
#ifdef DEBUG
									{
										char m[] = {mode,0};
										debug("Consule output was enabled (mode=",m,")");
									}
#endif
									controller.console(true);
									break;

								default:
									if(mode >= '1' && mode <= '9') {
										int value =  ((int) (mode-'0'))+3;
										debug("Setting console output to level ",value);
										controller.verbosity(value);
									} else {
										throw logic_error("Unexpected argument parser mode");
									}
							}

						}

						return ArgumentParser::Handled;
					}
				},
				ArgumentParser::Argument{
					'q', "quiet", _( "Disable console output" ),
					[](const char *, char) {
						Logger::Controller::getInstance().console(false);
						return ArgumentParser::NotHandled;
					}
				},
				ArgumentParser::Argument{
					'l', "logfile", _( "Enable log to file" ),
					[](const char *argument, char) {
						if(!(argument && *argument)) {
							throw runtime_error(_( "Log to file requires a filename" ));
						}
						Logger::Controller::getInstance().file(argument);
						return ArgumentParser::Handled;
					}
				},
				ArgumentParser::Argument{
					'L', "loglevel", _( "Set the log verbosity level" ),
					[](const char *argument, char) {
						if(argument && *argument) {
							Logger::Controller::getInstance().verbosity(argument);
						}
						return ArgumentParser::Handled;
					}
				}
		);

#ifndef _WIN32
		add_application_argument(
			ArgumentParser::Argument{
				'C', "coredump", _( "Enable coredump" ),
				[](const char *pattern, char) {

					debug("Enabling coredump");

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
					if(setrlimit(RLIMIT_CORE, &core_limits)) {
						throw system_error(errno,system_category(),"Unable to activate coredump");			
					}

					ArgumentParser::Result rc = ArgumentParser::NotHandled;
					if(pattern && *pattern) {
						// Set corepattern
						std::filebuf fb;
						fb.open("/proc/sys/kernel/core_pattern",std::ios::out);
						if(fb.is_open()) {
							std::ostream os{&fb};
							os << pattern << "\n";
							fb.close();
							Logger::String{"Coredump enabled using pattern '",pattern,"'"}.info("debug");
						} else {
							Logger::String{"Unable to set coredump pattern"}.error("debug");
						}
						rc = ArgumentParser::Handled;
					} else {

						std::ifstream file("/proc/sys/kernel/core_pattern");
						if(file.is_open()) {

							String line;
							getline(file,line,'\0');
							line.strip();
							file.close();
							Logger::String{"Coredump enabled using pattern '",line.c_str(),"'"}.info("debug");

						} else {

							Logger::String{"Coredump enabled, no pattern info"}.warning("debug");

						}

					}

					debug("Coredump enabled!");
					return rc;
				}
			}
		);
#endif // !_WIN32

		return *this;
	}

 }

