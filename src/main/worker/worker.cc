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

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static ModuleInfo moduleinfo;

	Worker::Worker(const char *n, const ModuleInfo *i) : name(n), info(i) {
		Controller::getInstance().insert(this);
	}

	Worker::Worker(const char *name) : Worker(name,&moduleinfo) {
	}

	Worker::~Worker() {
		Controller::getInstance().remove(this);
	}

	void Worker::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	bool Worker::work(const char *name,Request &request, Response &response) {

		bool rc = Worker::Controller::getInstance().find(name)->work(request,response);

		if(response.isNull()) {
			throw system_error(ENODATA,system_category(),"Empty response");
		}

		return rc;

	}

	void Worker::get(Request &request, Response &response) const {
		throw system_error(ENODATA,system_category(),"No 'get' method on this worker");
	}

	bool Worker:: work(Request &request, Response &response) const {
		if(request == Request::Type::Get) {
			get(request,response);
			return true;
		}
		return false;
	}

	size_t Worker::hash() const {

		// https://stackoverflow.com/questions/7666509/hash-function-for-string
		size_t value = 5381;

		for(const char *ptr = name; *ptr; ptr++) {
			value = ((value << 5) + value) + tolower(*ptr);
		}

		return value;

	}

}

