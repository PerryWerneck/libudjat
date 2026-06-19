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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/unit-test.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <udjat/ui/menu.h>
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
 #endif // !_WIN32

 using namespace std;
 using Console = Udjat::UI::Console;

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
				Logger::String{"Loading ",info->dlpi_name}.info();
				container->append_module(hModule,info->dlpi_name);
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

	void * UnitTests::Module::get_symbol(const char *symbol_name) {
#ifdef _WIN32
		void * symbol = (void *) GetProcAddress(handle,symbol_name);
		if(!symbol) {
			throw Win32::Exception(string{"Can't find symbol '"} + symbol_name + "'");
		}
		return symbol;
#else
		dlerror();
		void *symbol = dlsym(handle,symbol_name);
		const char *error = dlerror();
		if(error) {
			throw runtime_error(error);
		}
		return symbol;
#endif		
	}

	void UnitTests::load() noexcept {
#ifndef _WIN32
		Logger::String{"Scanning loaded modules"}.info();
		dl_iterate_phdr(phdr_item, this);
		Logger::String{"Found ",modules.size()," modules with unit tests"}.info();
#endif // !_WIN32

		// Load tests.
		for(auto &module : modules) {
			debug("Calling ",module->c_str(),"...");
			module->getfunc<void,UnitTests &>("enum_udjat_unit_tests")(*this);
			debug("--> ",size());
		} 

		Logger::String{"Found ",size()," tests to run"}.info();

	}

	UnitTests::UnitTests() {

	}

	UnitTests::~UnitTests() {
	}

	void UnitTests::run_all() noexcept {

		for(const auto &worker : workers ) {
			try {

				Logger::String{"--- ",worker.c_str()," ---"}.notice();
				worker.call();

			} catch(const std::exception &e) {

				Logger::String{worker.c_str(),": ",e.what()}.error();

			} catch(...) {

				Logger::String{worker.c_str(),": Unexpected error"}.error();

			}
		}
	}

	void UnitTests::interactive() noexcept {

		// Run interactive mode.
		while(1) {
			
			size_t selected = (size_t) -1;
			try {

				Console console;
				auto menu = console.menu(_("Available tests"));
				for(const auto &test : *this) {
					menu->append(test.c_str());	
				}
				selected = menu->select();

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

