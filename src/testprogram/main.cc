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
 #include <udjat/tools/url.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/commandlineparser.h>
 #include <string>
 #include <udjat/ui/console.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif

 using namespace Udjat;
 using namespace std;

 int main(int argc, char **argv) {

	static std::shared_ptr<UI::Animation> animations[] = {
		UI::Animation::Factory(UI::Animation::Style::PlainText),
		UI::Animation::Factory(UI::Animation::Style::Simple),
		UI::Animation::Factory(UI::Animation::Style::Braille),
		UI::Animation::Factory(UI::Animation::Style::Circle),
	};

	for(size_t count = 0; count < 100; count++) {
		cout << '\r';
		for(size_t ix = 0; ix < sizeof(animations)/sizeof(animations[0]); ix++) {
			cout << *animations[ix] << " ";
		}
		cout << " " << count << flush;
		usleep(500000);
	}

	/*
	// Call the loader function with command line arguments
	return loader(argc, argv,[](Application &app) -> int {
#ifdef TEST_PROGRAM
		return run_unit_test(nullptr);
#else
		return 0;
#endif // TEST_PROGRAM
		return 0;
	}, "test.xml");
	*/

 }
