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
 #include "private.h"

 namespace Udjat {

	/// @brief Connect to host.
	/// @return Socket connected to host.
	int URL::connect(time_t timeout) const {
		return protocol->connect(*this,timeout);
	}

	std::shared_ptr<URL::Response> URL::call(const Method method, const char *mimetype, const char *payload) {
		return protocol->call(*this,method,mimetype,payload);
	}

	std::shared_ptr<URL::Response> URL::call(const char *mname, const char *mimetype, const char *payload) {
		return protocol->call(*this,Method(mname),mimetype,payload);
	}

	std::shared_ptr<URL::Response> URL::get(const char *mimetype) const {
		return protocol->call(*this,URL::Method::Get,mimetype);
	}

	std::shared_ptr<URL::Response> URL::post(const char *payload, const char *mimetype) const {
		return protocol->call(*this,URL::Method::Post,mimetype,payload);
	}

 }

