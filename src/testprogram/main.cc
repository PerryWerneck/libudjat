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
 #include <udjat/ui/animation.h>
 #include <udjat/ui/menu.h>
 #include <udjat/tools/argumentparser.h>
 #include <thread>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif

 using namespace Udjat;
 using namespace std;

 int main(int argc, const char **argv) {

	{
		Logger::verbosity(9);
		Logger::console(true);

		try {
			ArgumentParser{
				"First group of options",
					ArgumentParser::Argument{
						'a', "opta", "Argument 'A'",
						[](const char *argument) {
							cout << "Argument A called" << endl;
							return false;
						}
					},
				
				"Second group of options",
					ArgumentParser::Argument{
						'b', "optb", "Argument 'B'",
						[](const char *argument) {
							cout << "Argument B called" << endl;
							return false;
						}
					},
					ArgumentParser::Argument{
						'c', "oc", "Argument 'C'",
						[](const char *argument) {
							cout << "Argument C called" << endl;
							return false;
						}
					}
			}.parse(argc,argv);
			
		} catch(const std::exception &e) {
			cerr << e.what() << endl;
			return 1;
		}
		return 0;
	}


	/*
#ifndef _WIN32
	Logger::file("/tmp/test.log");
	Logger::console(true);
	Logger::redirect();

	cout << "teste\tChecking log redirections" << endl;

	for(size_t ix = 0; ix < 5; ix++) {
		thread([ix](){
			for(size_t a = 0; a < (6-ix); a++) {
				cout << "teste" << ix << "\tChecking thread " << ix << "." << a<< endl;
			}
		}).detach();
	}

	sleep(5);
#endif
	*/

	/*
#ifndef _WIN32
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

	exit(0);
#endif
	*/

	/*
	{
		Value array;

		for(size_t ix = 0; ix < 10;ix++) {
			auto &row = array.append(Value::Object);
			row["id"] = ix;
			row["name"] = "name";
			row["description"] = "description";
			row["valid"] = true;
		}

		cout << endl;
		array.to_text(cout);
		cout << endl;

		exit(0);
	}
	*/

	/*
	{
		static const char *options[] = {
			"Option A",
			"Option B",
			"Option C",			
			"Option D",			
			"Option E",			
			"Option F",			
			"Option G",			
			"Option H",			
			"Option I",			
		};

		size_t selected = (size_t) -1;
		{
			UI::Console console;
			auto menu = console.menu("Title");
			menu->lines_per_page(5);

			for(const char *option : options) {
				menu->append(option);
			}

			try {

				selected = menu->select();

			} catch(const std::exception &e) {

				Logger::String{e.what()}.info();

			}
		}

		if(selected != (size_t) -1) {
			Logger::String{"Selected option: '",options[selected],"'"}.info();
		}

	}
	*/
	
	/*
	// Call the loader function with command line arguments
	return loader(argc, argv,[](Application &app) -> int {
// #ifdef TEST_PROGRAM
// 		return run_unit_test(nullptr);
//#else
//		return 0;
//#endif // TEST_PROGRAM
		return 0;
	}, "test.xml");
	*/

 }
