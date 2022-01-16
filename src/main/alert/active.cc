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

 #include "private.h"
 #include <udjat.h>
 #include <udjat/tools/timestamp.h>

 namespace Udjat {

	static void expander(string &text) {
		expand(text, [](const char *key){
			if(!strcasecmp(key,"timestamp")) {
				return TimeStamp().to_string(TIMESTAMP_FORMAT_JSON);
			}
			return string{"${}"};
		});
	}

	Alert::Controller::Active::Active(Alert *a) : alert(a), name(alert->settings.name), url(alert->url()), payload(alert->payload()) {
		expander(url);
		expander(payload);
		cout << name << "\tActivating alert " << url << endl;
	}

 }
