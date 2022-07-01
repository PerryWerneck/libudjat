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
 #include <udjat/tools/file.h>
 #include <udjat/moduleinfo.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 namespace Udjat {

	static const ModuleInfo moduleinfo { "File protocol module" };

	Protocol::Controller::File::File() : Udjat::Protocol((const char *) "file",moduleinfo) {
	}

	Protocol::Controller::File::~File() {
	}

	std::shared_ptr<Protocol::Worker> Protocol::Controller::File::WorkerFactory() const {

		class Worker : public Protocol::Worker {
		public:
			Worker() = default;

			string path() const {

				if(strncasecmp(url().c_str(),"file://.",8) == 0) {
					return url().c_str()+7;
				}

				return url().ComponentsFactory().path.c_str();
			}

			String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) {
				return String(Udjat::File::Text(path()).c_str());
			}

			unsigned short test() override {

				auto filepath = path();

				if(access(filepath.c_str(),R_OK) == 0) {
					return 200;
				}

				if(access(filepath.c_str(),F_OK) != 0) {
					return 404;
				}

				return -1;

			}

		};

		return make_shared<Worker>();
	}

	String Protocol::Controller::File::call(const URL &url, const HTTP::Method method, const char UDJAT_UNUSED(*payload)) const {
		if(method != HTTP::Get) {
			throw system_error(EINVAL,system_category(),"Invalid request method");
		}
		return String(Udjat::File::Text(url.ComponentsFactory().path.c_str()).c_str());
	}

 }

