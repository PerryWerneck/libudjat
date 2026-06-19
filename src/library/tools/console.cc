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


 // References:
 //
 // https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
 //

#include <config.h>
#include <udjat/ui/console.h>
#include <udjat/ui/menu.h>
#include <stdexcept>
#include <errno.h>
#include <udjat/tools/string.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/intl.h>

using namespace std;

namespace Udjat {

	std::shared_ptr<Dialog::Menu> UI::Console::menu(const char *title) {

		class Menu : public Dialog::Menu {
		private:
			Console *cntl;

		public:
			Menu(Console *c, const char *t) : Dialog::Menu{t}, cntl{c} {
				lpp = 15;
			}

			size_t select() override {	

				Console &console = *cntl;

				if(size() == 0) {
					throw system_error(ENODATA,system_category());
				}

				size_t page = 0;
				while(1) {

					console << endl;
					console.bold(true);
					console << title << endl;
					console.bold(false);
					console << endl;

					char first = 'A';

					bool next = true;
					char item[] = { first, '\0'};
					size_t lines = 5;

					for(size_t ix = 0; ix < lpp; ix++) {
						auto line = page*lpp+ix;

						if(line >= size()) {
							next = false;
							break;
						}

						lines++;
						auto &option = (*this)[line];
						console << "\t";
						console.bold(true);
						console << item;
						console.bold(false);
						console << " - " << option.c_str() << endl;
						item[0]++;				

					}

					if(next || page) {

						lines++;
						console << "\t";
						console.faint(true);
						if(page) {
							console << "< " << _("Previous page") << "   ";
						}
						if(next) {
							console << "> "<< _("Next page");
						}
						console.faint(false);

						console << endl;

					}


					console << endl << _("Select option (Enter to quit): ");
					console.cursor(true).flush();
					cin.sync();

					String choice;
					getline(cin,choice);
					choice.strip();
					
					for(size_t line = 0; line < lines;line++) {
						console.erase_line().up();
					}

					if(choice.empty()) {
						throw system_error(ECANCELED,system_category());
					}

					choice[0] = toupper(choice[0]);

					if(next && choice[0] == '>') {
						page++;
						continue;
					}

					if(page && choice[0] == '<') {
						page--;
						continue;
					}

					int selected = (choice[0] - first);
					if(selected < 0 || selected >= (int) lpp) {
						Logger::String{"Invalid option: '",choice,"'"}.warning("menu");
						continue;
					}
					
					selected += (page * lpp);
					if(selected >= (int) size()) {
						Logger::String{"Invalid option: '",choice,"'"}.warning("menu");
						continue;
					}

					Logger::String{"Option '",(*this)[selected],"' was selected"}.info("menu");
					return (size_t) selected;

				}

			}

		};

		return make_shared<Menu>(this,title);

	}

}
