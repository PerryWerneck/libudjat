/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/url.h>
 #include <string>
 #include <udjat/net/interface.h>

 using namespace Udjat;
 using namespace std;

 #ifdef HAVE_SMBIOS
 static int smbios_test() {
	try {
		string smbios = URL{"dmi:///BIOS"}.get();
		Logger::String{"SMBIOS information: ",smbios.c_str()}.info();
	} catch(const std::exception &e) {
		Logger::String{"Error getting SMBIOS information: ",e.what()}.error();
	}	
	return 0;
 }
 #endif // HAVE_SMBIOS

 static int network_test() {
	auto nic = Udjat::Network::Interface::Default();

	auto name = nic->name();
	if(name && *name) {
		Logger::String{"Default network interface address: ",name}.info();
	} else {
		throw logic_error("No default network interface found.");
	}	

	auto addr = nic->address().to_string();
	if(addr.empty()) {
		throw logic_error("No default network interface found.");
	} else {
		Logger::String{"Default network interface address: ",addr.c_str()}.info();
	}	

	auto mask = nic->netmask().to_string();
	if(mask.empty()) {
		throw logic_error("No default network interface netmask found.");
	} else {
		Logger::String{"Default network interface netmask: ",mask.c_str()}.info();
	}
	return 0;
 }

 UDJAT_API int run_unit_test(const char *name) {

	static const struct {
		const char *name;
		int (*test)();
	} tests[] = {
 #ifdef HAVE_SMBIOS
		{"smbios", smbios_test},
 #endif // HAVE_SMBIOS
		{"network", network_test},
	};

	if(!name) {
		for(const auto &test : tests) {
			Logger::String{"Running unit test: ",test.name}.info();
			test.test();
		}
	} else {
		for(const auto &test : tests) {
			if(strcasecmp(test.name, name) == 0) {
				Logger::String{"Running unit test: ",test.name}.info();
				return test.test();
			}
		}
	}

	return 0;
 }

 
 