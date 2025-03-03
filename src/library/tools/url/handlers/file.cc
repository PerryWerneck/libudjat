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
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/logger.h>
 #include <private/url.h>
 #include <stdexcept>
 #include <errno.h>

 #ifdef _WIN32
	#include <shlwapi.h>
 #endif

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	const char * FileURLHandler::c_str() const noexcept {
		return path.c_str();
	}

	int FileURLHandler::perform(const HTTP::Method, const char *, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) {

		debug("-----------> Loading '",path.c_str(),"'");

		File::Handler file{path.c_str()};

		size_t block_size = file.block_size();
		char buffer[block_size];

		uint64_t total = file.length();
		uint64_t current = 0;

		while(current < total) {

			size_t len = file.read((void *) buffer,block_size,false);
			if(!len) {
				break;
			}

			if(progress(current,total,buffer,len)) {
				Logger::String{"Loading of '",path.c_str(),"' was canceled"}.trace();
				throw system_error(ECANCELED,system_category());
			}

			current += len;

		}

		return 200;

	}

	int FileURLHandler::test(const HTTP::Method, const char *) {

#ifdef _WIN32
		if(!PathFileExists(path.c_str())) {
			return 404;
		}

		return 200;
#else
		if(access(path.c_str(),R_OK) == 0) {
			return 200;
		}

		if(access(path.c_str(),F_OK) != 0) {
			return 404;
		}

		return 401;
#endif // _WIN32

	}


/*

 #include <private/protocol.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/intl.h>
 #include <udjat/module/info.h>

 #ifdef _WIN32
	#include <shlwapi.h>
 #else
	#include <unistd.h>
 #endif // _WIN32
	Protocol & Protocol::FileHandlerFactory() {

		static const ModuleInfo moduleinfo { N_( "File protocol" ) };

		class FileWorker : public Protocol::Worker {
		public:
			FileWorker() = default;

			string path() const {

				if(strncasecmp(url().c_str(),"file://",7) == 0) {
					return url().c_str()+7;
				}

				return url().ComponentsFactory().path.c_str();
			}

			String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) override {
				return String(Udjat::File::Text(path()).c_str());
			}

			bool save(const char *filename, const std::function<bool(double current, double total)> &progress, bool replace) override {
				Udjat::File::copy(path().c_str(),filename,progress,replace);
				return true;
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

				if(method() != HTTP::Head) {
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

		class Handler : public Udjat::Protocol {
		public:
			Handler() : Udjat::Protocol((const char *) "file",moduleinfo) {
			}

			~Handler() {
			}

			String call(const URL &url, const HTTP::Method method, const char *) const override {
				if(method != HTTP::Get) {
					throw system_error(EINVAL,system_category(),"Invalid request method");
				}
				return String(Udjat::File::Text(url.ComponentsFactory().path.c_str()).c_str());
			}

			std::shared_ptr<Protocol::Worker> WorkerFactory() const {
				return make_shared<FileWorker>();
			}

		};

		static Handler instance;
		return instance;

	}
*/

 }

