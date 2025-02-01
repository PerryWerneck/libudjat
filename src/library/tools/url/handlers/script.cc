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

 #ifndef _WIN32
	#include <signal.h>
 #endif

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 namespace Udjat {

	int ScriptURLHandler::perform(const HTTP::Method, const char *, const std::function<bool(uint64_t current, uint64_t total, const char *data, size_t len)> &progress) {

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
				this->rc = -sig;
			}

		};

		int rc = test();
		if(rc != 200) {
			return rc;
		}

		rc = Worker{this->path().c_str(),progress}.run();
		if(rc == 0) {
			return 200;
		}

		return rc;

	}

	int ScriptURLHandler::test(const HTTP::Method, const char *) {

		String path{this->path()};

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
 #include <config.h>
 #include <private/protocol.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/intl.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/intl.h>
 #include <cstdio>
 #include <iostream>
 #include <sstream>
 #include <fcntl.h>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	Protocol & Protocol::ScriptHandlerFactory() {

		static const ModuleInfo moduleinfo { N_("Subprocess/Script protocol" ) };

		/// @brief Script worker.
		class ScriptWorker : public Protocol::Worker {
		private:

			/// @brief Get script path, download it to cache if necessary.
			string path() const {

				const char *url = this->url().c_str();

				if(strncasecmp(url,"script+",7) == 0) {

					// TODO: Download URL+7, save on cache.

					throw system_error(ENOTSUP,system_category(),"Script from URL is not implemented");

				}

				auto components = this->url().ComponentsFactory();

				if(components.remote()) {
					throw system_error(EINVAL,system_category(),"Dont know hot to handle remote scripts");
				}

				return components.path;

			}


		public:
			ScriptWorker() = default;

			/// @brief Run script, capture output.
			String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) {

				string script = this->path();

				if(access(script.c_str(),F_OK)) {
					throw system_error(ENOENT,system_category(),script);
				}

				if(access(script.c_str(),R_OK)) {
					throw system_error(EPERM,system_category(),script);
				}

				/// @brief Run script, capture output to string.
				class Process : public SubProcess, public stringstream {
				public:
					int rc = 0;
					Process(const char *command) : SubProcess(command) {
					}

				private:
					void onStdOut(const char *line) override {
						*this << line << endl;
					}

					void onStdErr(const char *line) override {
						*this << line << endl;
					}

					void onExit(int rc) override {
						this->rc = rc;
					}

					void onSignal(int UDJAT_UNUSED(sig)) override {
						this->rc = -1;
					}

				};

				Process proc(script.c_str());
				proc.run();

				return String(proc.str());

			}

			/// @brief Test if script file is available.
			int test(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) noexcept override {

				if(method() != HTTP::Head) {
					return EINVAL;
				}

				// Get file path, download it if necessary.
				string script = this->path();

				if(access(script.c_str(),F_OK)) {
					// File not found, return 404.
					clog << "script\t" << script << " is not available" << endl;
					return 404;
				}

				if(access(script.c_str(),R_OK)) {
					// Cant read from file, return 401.
					clog << "script\t" << script << " is not acessible" << endl;
					return 401;
				}

				// Run script.
				int rc = SubProcess::run(script.c_str());

				if(rc == 0) {
					return 200;
				}

				// Failed, return 500.
				return 500;
			}

		};

		class Handler : public Udjat::Protocol {
		public:
			Handler() : Udjat::Protocol((const char *) "script",moduleinfo) {
			}

			~Handler() {
			}

			std::shared_ptr<Protocol::Worker> WorkerFactory() const {
				return make_shared<ScriptWorker>();
			}

		};

		static Handler instance;
		return instance;

	}


*/

 }

