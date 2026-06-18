/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
#include <udjat/tools/loader.h>
#include <udjat/tools/argumentparser.h>
#include <udjat/tools/application.h>
#include <udjat/tools/intl.h>
#include <stdexcept>

#ifndef _WIN32
	#include <dlfcn.h>
	#include <link.h>
#endif // !_WIN32

using namespace std;

namespace Udjat {

#ifndef _WIN32
	static int phdr_item(struct dl_phdr_info *info, size_t size, void *data) {

		if(!info->dlpi_name || !*info->dlpi_name) {
			debug("Skipping main program");
			return 0;
		}

		debug("Name: ",info->dlpi_name);
		void *hModule = dlopen(info->dlpi_name, RTLD_NOW|RTLD_LOCAL);
		if(hModule) {
			size_t *count = (size_t *) data;
			dlerror(); // Clear any existing error
			int (*symbol)(const char *) = (int(*)(const char *)) dlsym(hModule,"run_udjat_unit_test");
			auto error = dlerror();
			if(symbol && !error) {
				(*count)++;
				Logger::String{"------------- Running unit tests from module '",info->dlpi_name,"' -------------"}.notice("debug");
				try {
					int rc = symbol(nullptr);
					if(rc) {
						dlclose(hModule);
						return rc;	
					}
				} catch(const std::exception &e) {
					Logger::String{"Error running unit tests from module '",info->dlpi_name,"': ",e.what()}.error();
					dlclose(hModule);
					return -1;
				}
			} else {
				debug(error ? error : "No unit tests found in module");
			}
			dlclose(hModule);
		} else {
			Logger::String{"Error opening '",info->dlpi_name,"': ",dlerror()}.error("debug");
		}
	return 0;
}
#endif // !_WIN32

	/// @brief Detect and run unit tests from modules.
	/// @param name The test name
	static void run_unit_tests(const char *name) {
#ifndef _WIN32
		size_t count = 0;
		dl_iterate_phdr(phdr_item, &count);
#endif // !_WIN32
	}

	int UDJAT_API loader(const int argc, const char *argv[], const char *path) {
		return Udjat::loader(argc,argv,[](const LoaderMode, Application &, const char *) {return false;},path);
	}

	int UDJAT_API loader(const int argc, const char *argv[], const std::function<bool(const LoaderMode mode, Application &app, const char *arg)> &init, const char *path) {

		class Loader : public Udjat::Application {
		private:
			const std::function<bool(const LoaderMode mode, Application &app, const char *arg)> &callback;

		protected:
			ArgumentParser & load(ArgumentParser &parser) noexcept override {

				parser.append(
					ArgumentParser::Argument{
						't', "run-unit-tests", _("Run unit tests"),
						[this](const char *arg, char) {
							if(callback(LOADER_MODE_RUN_TESTS,*this,arg)) {
								return true;
							}
							run_unit_tests(arg);
							return true;
						}
					},
					ArgumentParser::Argument{
						'M', "load-module", _("Load module file"), _("path"),
						[](const char *path, char) {

							if(!(path && *path)) {
								throw runtime_error("Load module requires the module path as argument");
							}

							// TODO: Implement

							return false;
						}
					}
				);

				return parser;
			}

		public:
			Loader(const int argc, const char *argv[], const std::function<bool(const LoaderMode mode, Application &app, const char *arg)> &cbk) : Udjat::Application(argc,argv), callback(cbk) {
			}

			std::shared_ptr<Abstract::Agent> RootFactory() override {
				callback(LOADER_MODE_INIT,*this,"");
				return Udjat::Application::RootFactory();
			}

		};

		Logger::verbosity(9);
		Logger::console(true);

		return Loader{argc,argv,init}.run(path);

	}

	int UDJAT_API loader(const int argc, const char *argv[], const std::function<int(Application &app)> &init, const char *path) {

		class Loader : public Udjat::Application {
		private:
			const std::function<int(Application &app)> &callback;

		protected:
			ArgumentParser & load(ArgumentParser &parser) noexcept override {

				parser.append(
					ArgumentParser::Argument{
						't', "run-unit-tests", _("Run unit tests"),
						[this](const char *arg, char) {
							run_unit_tests(arg);
							return true;
						}
					},
					ArgumentParser::Argument{
						'M', "load-module", _("Load module from file"), _("path"),
						[](const char *path, char) {

							if(!(path && *path)) {
								throw runtime_error("Load module requires the module path as argument");
							}

							// TODO: Implement

							return false;
						}
					}
				);

				return parser;
			}

		public:
			Loader(const int argc, const char *argv[], const std::function<int(Application &app)> &cbk) : Udjat::Application(argc,argv), callback(cbk) {
			}

			std::shared_ptr<Abstract::Agent> RootFactory() override {
				if(callback(*this)) {
					throw runtime_error{"Initialization failed"};
				}
				return Udjat::Application::RootFactory();
			}

		};

		Logger::verbosity(9);
		Logger::console(true);

		return Loader{argc,argv,init}.run(path);

	}

}