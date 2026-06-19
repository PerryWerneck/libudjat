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
 #include <udjat/tools/application.h>
 #include <udjat/tools/argumentparser.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 
 using namespace std;

 namespace Udjat {

	ArgumentParser & Application::load(ArgumentParser &parser) noexcept {

		parser.add_application_argument(
			ArgumentParser::Argument{
				'T', "timer", _("Enable auto exit timer in seconds"), _("seconds"),
				[](const char *seconds, char) {

					if(!seconds && *seconds) {
						throw runtime_error(_("Timer option requires a value in seconds"));
					}

					MainLoop::getInstance().TimerFactory(((time_t) TimeStamp{seconds}) * 1000,[](){
						MainLoop::getInstance().quit("Timer expired, exiting");
						return false;
					});

					return false;
				}
			});

		return parser;
	}

 }

