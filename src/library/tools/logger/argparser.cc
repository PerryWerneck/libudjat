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
				[](const char *argument, char) {
					return false;
				}
			}
		);
#endif // !_WIN32

		return *this;
	}

 }

