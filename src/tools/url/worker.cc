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

	Protocol::Worker & Protocol::Worker::credentials(const char UDJAT_UNUSED(*user), const char UDJAT_UNUSED(*passwd)) {
		throw system_error(ENOTSUP,system_category(),"No credentials support on selected worker");
	}

	Protocol::Header & Protocol::Worker::header(const char UDJAT_UNUSED(*name)) {
		throw system_error(ENOTSUP,system_category(),string{"The selected worker was unable do create header '"} + name + "'");
	}

	static const std::function<bool(double current, double total)> dummy_progress([](double UDJAT_UNUSED(current), double UDJAT_UNUSED(total)) {
		return true;
	});

	String Protocol::Worker::get() {
		return get(dummy_progress);
	}

	bool Protocol::Worker::save(const char *filename) {
		return save(filename, dummy_progress);
	}

 }


