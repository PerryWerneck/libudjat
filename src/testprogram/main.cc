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
 #include <udjat/tools/variant.h>
 #include <iostream>

 using namespace Udjat;
 using namespace std;

 int main(int argc, char **argv) {

	Logger::verbosity(9);
	Logger::console(true);
	
	Variant response;
	response["provider"] = "provider";
	auto &k7 = response["k7"];
	k7.clear(Variant::Array);

	for(size_t ix = 0 ; ix < 10; ix++) {
		auto &row = k7.append();
		row["ix"] = ix;
		row["text"] = "text";
	}

	cout << "--------------------------------" << endl;
	response.to_yaml(cout);
	cout << "--------------------------------" << endl;

	// return loader(argc, argv, [](const LoaderMode mode, Application &app, const char *arg){
	// 	return 0;
	// });

 }
