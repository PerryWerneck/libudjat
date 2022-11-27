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
 #include <private/protocol.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/intl.h>
 #include <udjat/moduleinfo.h>

 #ifdef _WIN32
	#include <shlwapi.h>
 #else
	#include <unistd.h>
 #endif // _WIN32

 namespace Udjat {

	static const ModuleInfo moduleinfo { N_( "Local file protocol" ) };

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

			String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) override {
				return String(Udjat::File::Text(path()).c_str());
			}

			std::string filename(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) override {
				// No need for caching, just return the local file path.
				auto filepath = path();

				if(access(filepath.c_str(),F_OK) != 0) {
					throw system_error(ENOENT,system_category(),filepath);
				}

				return filepath;
			}

			int test(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) noexcept override {

				if(method() != HTTP::Get) {
					return EINVAL;
				}

				auto filepath = path();

#ifdef _WIN32
				if(!PathFileExists(filepath.c_str())) {
					return 404;
				}

				return 200;
#else
				if(access(filepath.c_str(),R_OK) == 0) {
					return 200;
				}

				if(access(filepath.c_str(),F_OK) != 0) {
					return 404;
				}

				return 401;
#endif // _WIN32


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

