/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements Request::exec.
  */

 #include <config.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response/value.h>
 #include <udjat/tools/response/table.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <private/worker.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API bool exec(Request &request, Response::Value &response) {
		return Worker::for_each([&request,&response](const Worker &worker) {
			if(worker.probe(request) & Worker::Value) {
				return worker.work(request,response);
			}
			return false;
		});
	}

	UDJAT_API bool exec(Request &request, Response::Table &response) {
		return Worker::for_each([&request,&response](const Worker &worker) {
			if(worker.probe(request) & Worker::Table) {
				return worker.work(request,response);
			}
			return false;
		});
	}

	UDJAT_API bool introspect(Udjat::Value &value) {
		bool rc = false;
		Worker::for_each([&value,&rc](const Worker &worker){
			if(worker.introspect(value)) {
				rc = true;
			}
			return false;
		});
		return rc;
	}

 }
