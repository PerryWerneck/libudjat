/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/subprocess.h>
 #include <private/url.h>
 #include <errno.h>

 #ifdef _WIN32
	#include <shlwapi.h>
 #else
	#include <signal.h>
 #endif

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 namespace Udjat {

	const char * ScriptURLHandler::c_str() const noexcept {
		return path.c_str();
	}

	int ScriptURLHandler::perform(const HTTP::Method, const char *, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) {

		/// @brief Run script, capture output to lambda.
		class Worker : public SubProcess {
		public:
			int rc = 0;
			size_t current = 0;
			const std::function<bool(uint64_t current, uint64_t total, const char *data, size_t len)> &progress;

			Worker(const char *command, const std::function<bool(uint64_t current, uint64_t total, const char *data, size_t len)> &p) : SubProcess{command}, progress{p} {
			}

		private:
			void onStdOut(const char *line) override {
#ifdef _WIN32

				progress(current,0,line,strlen(line));
#else
				if(progress(current,0,line,strlen(line))) {
					kill(SIGTERM,getpid());
				}

#endif
				current += strlen(line);
			}

			void onStdErr(const char *line) override {
#ifdef _WIN32
				progress(current,0,line,strlen(line));
#else
				if(progress(current,0,line,strlen(line))) {
					kill(SIGTERM,getpid());
				}
#endif
				current += strlen(line);
			}

			void onExit(int rc) override {
				this->rc = rc;
			}

			void onSignal(int sig) override {
				if(sig == SIGTERM) {
					this->rc = ECANCELED;
				} else {
					this->rc = -sig;
				}
			}

		};

		int rc = test();
		if(rc != 200) {
			return rc;
		}

		rc = Worker{path.c_str(),progress}.run();
		if(rc == 0) {
			return 200;
		}

		return rc;

	}

	int ScriptURLHandler::test(const HTTP::Method, const char *) {

#ifdef _WIN32
		if(!PathFileExists(path.c_str())) {
			return 404;
		}

		return 200;
#else
		if(access(path.c_str(),X_OK) == 0) {
			return 200;
		}

		if(access(path.c_str(),F_OK) != 0) {
			return 404;
		}

		return 401;
#endif // _WIN32

	}

 }

