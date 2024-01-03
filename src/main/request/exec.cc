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
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <private/worker.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API bool exec(Request &request, Response::Value &response) noexcept {

		try {

			if(!Worker::Controller::getInstance().exec<Response::Value>(request,response)) {
				response.failed(ENOENT);
				return false;
			}

		} catch(const std::system_error &e) {

			response.failed(e);

		} catch(const std::exception &e) {

			response.failed(e);

		} catch(...) {

			response.failed(_("Unexpected error processing request"));

		}

		return true;
	}

	UDJAT_API bool exec(Request &request, Response::Table &response) noexcept {

		try {

			if(!Worker::Controller::getInstance().exec<Response::Table>(request,response)) {
				response.failed(ENOENT);
			}

		} catch(const std::system_error &e) {

			response.failed(e);

		} catch(const std::exception &e) {

			response.failed(e);

		} catch(...) {

			response.failed(_("Unexpected error processing request"));

		}

		return true;

	}

	UDJAT_API bool introspect(Udjat::Value &value) {

		// TODO: Pending implementation.
		return false;

		/*
		bool rc = false;
		Worker::for_each([&value,&rc](const Worker &worker){

			return false;
		});

		return rc;
		*/
	}

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
