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
 #include <udjat/tools/unit-test.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <udjat/ui/menu.h>
 #include <udjat/ui/console/menu.h>
 #include <udjat/ui/console.h>
 #include <udjat/tools/intl.h>

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
			UnitTests *container = (UnitTests *) data;
			dlerror(); // Clear any existing error
			void (*symbol)(UnitTests &) = (void(*)(UnitTests &)) dlsym(hModule,"enum_udjat_unit_tests");
			auto error = dlerror();
			if(symbol && !error) {

				char filename[PATH_MAX+1];
				memset(filename,0,sizeof(filename));
				if (realpath(info->dlpi_name, filename) == NULL)  {

					Logger::String{info->dlpi_name,": ",strerror(errno)}.error();

				} else {

					debug("Found '",filename,"'");

					// TODO: Check if already loaded

					container->append_module(hModule,filename);
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

	UnitTests::Module::~Module() {
		debug("Releasing module ",c_str());
#ifndef _WIN32
		dlclose(handle);
#endif		
	}

#ifndef _WIN32
	void * UnitTests::Module::dlsym(const char *name) const noexcept {
		return ::dlsym(handle,name);
	}
#endif // !_WIN32

	bool UnitTests::Worker::operator==(const char *opt) const {

		if(option && *option && strcasecmp(option,opt) == 0) {
			return true;
		}

		if(label && *label && strcasecmp(label,opt) == 0) {
			return true;
		}

		return false;
	}

	void UnitTests::load() noexcept {

#ifdef _WIN32

		// Load tests from modules.
		debug("--- Analizing ",modules.size()," modules");
		Udjat::Module::for_each([this](Udjat::Module &module){
			auto *symbol = reinterpret_cast<void(*)(UnitTests &)>(module.get_symbol("enum_udjat_unit_tests",false));
			if(symbol) {
				symbol(*this);
			}
			return false;
		});

#else

		Logger::String{"Scanning loaded modules"}.info();
		dl_iterate_phdr(phdr_item, this);
		Logger::String{"Found ",modules.size()," modules with unit tests"}.info();

		for(auto module : modules) {
			dlerror();
			auto *symbol = reinterpret_cast<void(*)(UnitTests &)>(module->dlsym("enum_udjat_unit_tests"));
			if(!dlerror()) {
				debug("Found tests in ",module->c_str());
				symbol(*this);
			}
		}

#endif // _WIN32

		// Sort options
		std::sort(workers.begin(), workers.end(), [](const Worker& a, const Worker& b) {
			return strcasecmp(a.label,b.label) < 0;
		});

		// Remove duplicate
		auto it = std::unique(workers.begin(), workers.end(), [](const Worker& a, const Worker& b) {
			return strcasecmp(a.label, b.label) == 0; // Note: == 0 checks for equality
		});
		workers.erase(it, workers.end());		

	}

	UnitTests::UnitTests() {

	}

	UnitTests::~UnitTests() {
	}

	void UnitTests::run(const char *name) noexcept {

		for(const auto &worker : workers ) {
			try {

				if(!(name && *name) || worker == name) {
					Logger::String{"--- ",worker.c_str()," ---"}.notice();
					worker.call();
				}

			} catch(const std::exception &e) {

				Logger::String{worker.c_str(),": ",e.what()}.error();

			} catch(...) {

				Logger::String{worker.c_str(),": Unexpected error"}.error();

			}
		}
	}

	void UnitTests::interactive() noexcept {

		// Run interactive mode.
		Console::Menu<string> menu{_("Available tests")};
		{
			// Get widht
			size_t width = 0;
			for(const auto &worker : workers) {
				width = max(width,worker.size());
			}

			//
			for(const auto &worker : workers) {
				String opt{worker.c_str()};

				if(worker.option && *worker.option) {
					for(size_t ix = worker.size();ix < width;ix++) {
						opt.append(" ");
					}
					opt.append(
						"  \x1B[2m",
						"( -r ",worker.option," )",
						"\x1B[22m"
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

			debug("selected options '",selected,"'");

			auto &worker = workers[selected];
			Logger::String{"--- ",worker.c_str()," ---"}.notice();

			try {

				worker.call();

			} catch(const std::exception &e) {
				Logger::String{e.what()}.error();
			}

		}

	}


 }

