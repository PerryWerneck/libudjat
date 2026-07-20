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

 #define LOG_DOMAIN "test"

 #ifndef _GNU_SOURCE
        #define _GNU_SOURCE             /* See feature_test_macros(7) */
 #endif // _GNU_SOURCE

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/testsuite.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <udjat/ui/menu.h>
 #include <udjat/ui/console/menu.h>
 #include <udjat/ui/console.h>
 #include <udjat/tools/intl.h>
 #include <algorithm>
 #include <list>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef _WIN32
	#include <udjat/win32/exception.h>
 #else
	#include <dlfcn.h>
	#include <link.h>
	#include <fcntl.h>
	#include <sys/param.h>
 #endif // !_WIN32

 using namespace std;

 namespace Udjat {

#ifndef _WIN32
	static int phdr_item(struct dl_phdr_info *info, size_t size, void *data) {

		if(!info->dlpi_name || !*info->dlpi_name) {
			debug("Skipping main program");
			return 0;
		}

		void *hModule = dlopen(info->dlpi_name, RTLD_NOW|RTLD_LOCAL);
		if(hModule) {
			TestSuite *container = (TestSuite *) data;
			dlerror(); // Clear any existing error
			void (*symbol)(TestSuite &) = (void(*)(TestSuite &)) dlsym(hModule,"udjat_register_tests");
			auto error = dlerror();
			if(symbol && !error) {

				char filename[PATH_MAX+1];
				memset(filename,0,sizeof(filename));
				if (realpath(info->dlpi_name, filename) == NULL)  {

					Logger::String{info->dlpi_name,": ",strerror(errno)}.error();

				} else {

					debug("Found '",filename,"'");

					// TODO: Check if already loaded

					container->add(hModule,filename);
				}

			} else {
				dlclose(hModule);
			}
		} else {
			Logger::String{"Error opening '",info->dlpi_name,"': ",dlerror()}.error();
		}

		return 0;
	}
#endif // !_WIN32

	TestSuite::Module::~Module() {
		debug("Releasing module ",c_str());
#ifndef _WIN32
		dlclose(handle);
#endif		
	}

#ifndef _WIN32
	void * TestSuite::Module::dlsym(const char *name) const noexcept {
		return ::dlsym(handle,name);
	}
#endif // !_WIN32

	bool TestSuite::Case::operator==(const char *opt) const {

		if(option && *option && strcasecmp(option,opt) == 0) {
			return true;
		}

		if(label && *label && strcasecmp(label,opt) == 0) {
			return true;
		}

		return false;
	}

	void TestSuite::load() noexcept {

#ifdef _WIN32

		// Load tests from modules.
		debug("--- Analizing ",modules.size()," modules");
		Udjat::Module::for_each([this](Udjat::Module &module){
			auto *symbol = reinterpret_cast<void(*)(UnitTests &)>(module.get_symbol("udjat_register_tests",false));
			if(symbol) {
				symbol(*this);
			}
			return false;
		});

#else

		dl_iterate_phdr(phdr_item, this);
		for(auto module : modules) {
			dlerror();
			auto *symbol = reinterpret_cast<void(*)(TestSuite &)>(module->dlsym("udjat_register_tests"));
			if(!dlerror()) {
				debug("Found tests in ",module->c_str());
				symbol(*this);
			}
		}

#endif // _WIN32

	}

	TestSuite::TestSuite(const char *title) {
		add(title && *title ? title : "Available tests");
	}

	TestSuite::~TestSuite() {
	}

	void TestSuite::add(const Case &obj) {
		for(auto &cs : groups.back().cases) {
			if(cs == obj) {
				// Same case, ignore it.
				return;
			}
		}
		groups.back().cases.push_back(obj);
	}

	void TestSuite::run(const char *path) noexcept {

		// TODO: Refactor using groups.
		throw runtime_error("Incomplete");
		
		/*
		for(const auto &worker : workers ) {
			try {

				if(!(name && *name) || worker == name) {
					
					Logger::String{"--- ",worker.c_str()," ---"}.notice();
					auto result = worker.call(std::cout);

					Console::status(Logger::Info,worker.c_str(),result.c_str());

				}

			} catch(const std::exception &e) {

				Console::status(Logger::Error,worker.c_str(),e.what());

			} catch(...) {

				Console::status(Logger::Error,worker.c_str(),"Unexpected error");

			}

		}
		*/

	}

	void TestSuite::for_each(const std::function<void(const char *option, const char *label)> &func) const {
		for(const auto &group : groups) {
			for(const auto &testcase : group.cases) {
				func(testcase.option,testcase.label);
			}
		}
	}

	void TestSuite::Group::interactive() noexcept {

		Console::Menu<string> menu{title};
		{
			// Get width
			size_t width = 0;
			for(const auto &testcase : cases) {
				width = max(width,testcase.size());
			}

			//
			for(const auto &testcase : cases) {
				String opt{testcase.c_str()};

				if(testcase.option && *testcase.option) {
					for(size_t ix = testcase.size();ix < width;ix++) {
						opt.append(" ");
					}
					opt.append(
						"  ",Console::SetFaint,
						"(",testcase.option,")",
						Console::ResetFaint
					);
				}

				menu.push_back(opt);	
			}
		}

		while(1) {
			
			size_t selected = (size_t) -1;
			try {

				selected = menu.select();

			} catch(const std::exception &e) {
				Logger::String{e.what()}.error();
				return;
			}

			auto &testcase = cases[selected];

			try {

				auto status = testcase.call(std::cout);
				Console::status(Logger::Info,testcase.c_str(),status.c_str());

			} catch(const std::exception &e) {
				Console::status(Logger::Error,testcase.c_str(),e.what());
			} catch(...) {
				Console::status(Logger::Error,testcase.c_str(),"Unexpected error");
			}

		}

	}

	void TestSuite::interactive() noexcept {

		// Strip empty groups.
		groups.remove_if([](Group &group){
			return group.cases.size() == 0;
		});

		// Run menu.
		if(groups.size() == 1) {

			// Just one group, run single mode.
			groups.begin()->interactive();

		} else {

			// TODO: Multiple groups, select one
			throw runtime_error("Incomplete");

		}

	}

 }

