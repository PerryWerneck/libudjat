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
 #include <cstring>

 namespace Udjat {

	Protocol::Worker::Worker(const char *url, const HTTP::Method method, const char *payload) {
		args.url = url;
		args.method = method;
		args.payload = payload;

		if(method == HTTP::Get && !args.payload.empty()) {
			clog << "protocol\tUnexpected payload on '" << method << "' " << url << endl;
		}
	}

	Protocol::Worker::~Worker() {
	}

	Protocol::Header & Protocol::Worker::header(const char UDJAT_UNUSED(*name)) {
		throw runtime_error(string{"Cant add headers to "} + args.url);
	}

	bool Protocol::Worker::header(const char *name, const char *value) {
		return header(name).assign(value);
	}

	String Protocol::Worker::get() {
		return get([](double UDJAT_UNUSED(current), double UDJAT_UNUSED(total)){return true;});
	}

	bool Protocol::Worker::save(const char *filename) {
		return save(filename,[](double UDJAT_UNUSED(current), double UDJAT_UNUSED(total)){return true;});
	}

 }


