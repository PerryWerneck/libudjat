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
 #include <udjat/tools/url.h>

 namespace Udjat {

	Alert::Worker::Worker(const char *n, const ModuleInfo *i) : name(n), info(i) {
		Alert::Controller::getInstance().insert(this);
	}

	Alert::Worker::~Worker() {
		Alert::Controller::getInstance().remove(this);
	}

	Alert::Worker::Worker(const char *n) : Worker(n,nullptr) {
	}

	void Alert::Worker::send(const Alert &alert, const string &url, const string &payload) const {
#ifdef DEBUG
		cout << "worker\tProcessing alert " << url << endl;
#endif // DEBUG

		auto response =
			Udjat::URL(url.c_str())
			.call(
				alert.action(),
				nullptr,
				payload.c_str()
			);

		if(response->failed()) {
			cout << alert.name() << "\t" << url << " " << response->getStatusCode() << " " << response->getStatusMessage() << endl;
			throw runtime_error(response->getStatusMessage());
		}

	}

 }
