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
 #include <udjat/loader.h>
 #include <iostream>
 #include <udjat/net/interface.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/logger.h>

 using namespace Udjat;
 using namespace std;

 int main(int argc, char **argv) {

	 // Call the loader function with command line arguments
	 return loader(argc, argv,[](Application &app) {

		debug("-------[ Beginning test of network methods]--------------");
		
		{
			auto nic = Udjat::Network::Interface::Default();

			auto name = nic->name();
			if(name && *name) {
				app.info() << "Default network interface address: " << name << endl;
			} else {
				throw logic_error("No default network interface found.");
			}	

			auto addr = nic->address().to_string();
			if(addr.empty()) {
				throw logic_error("No default network interface found.");
			} else {
				app.info() << "Default network interface address: " << addr.c_str() << endl;
			}	

			auto mask = nic->netmask().to_string();
			if(mask.empty()) {
				throw logic_error("No default network interface netmask found.");
			} else {
				app.info() << "Default network interface netmask: " << mask.c_str() << endl;
			}

#ifdef HAVE_SMBIOS
			try {
				string smbios = URL{"dmi:///BIOS"}.get();
				app.info() << "SMBIOS information: " << smbios.c_str() << endl;
			} catch(const std::exception &e) {
				app.error() << "Error getting SMBIOS information: " << e.what() << endl;
			}	

			// throw runtime_error("This is a test error to check the error handling in the application.");
#endif // HAVE_SMBIOS

		}

		debug("---------------------------------------------------------");

	}, "test.xml");

 }
