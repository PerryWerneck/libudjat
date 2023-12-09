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
 #include <udjat/tools/response.h>
 #include <udjat/tools/report.h>
 #include <udjat/tools/worker.h>
 #include <stdexcept>
 #include <private/worker.h>

 using namespace std;

 namespace Udjat {

	/*
	bool Request::exec(Response::Value &response, bool required) {

		const char *saved_path = reqpath;

		try {

			const Worker *selected_worker = nullptr;

			debug("Searching worker for '",reqpath,"'");

			bool found = Worker::for_each([this,&selected_worker](const Worker &worker){

				const char *path{worker.check_path(reqpath)};
				if(!path) {
					return false;
				}

				debug("Worker '",worker.c_str(),"' selected path '",path,"'");

				selected_worker = &worker;
				reqpath = path;

				return true;

			});

			if(found) {

				selected_worker->work(*this,response);

			} else if(required) {

				throw system_error(ENOENT,system_category(),Logger::String{"Cant handle '",c_str(),"'"});

			}

			return found;

		} catch(...) {

			reqpath = saved_path;
			throw;

		}

	}
	*/

	/*
	bool Request::exec(Response::Table &response, bool required) {

		const char *saved_path = reqpath;

		try {

			const Worker *selected_worker = nullptr;

			debug("Searching worker for '",reqpath,"'");

			bool found = Worker::for_each([this,&selected_worker](const Worker &worker){

				const char *path{worker.check_path(reqpath)};
				if(!path) {
					return false;
				}

				debug("Worker '",worker.c_str(),"' selected path '",path,"'");

				selected_worker = &worker;
				reqpath = path;

				return true;

			});

			if(found) {

				selected_worker->work(*this,response);

			} else if(required) {

				throw system_error(ENOENT,system_category(),Logger::String{"Cant handle '",c_str(),"'"});

			}

			return found;

		} catch(...) {

			reqpath = saved_path;
			throw;

		}

	}
	*/

 }
