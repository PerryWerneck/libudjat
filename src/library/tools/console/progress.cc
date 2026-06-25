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
#include <udjat/ui/console.h>
#include <udjat/ui/console/progress.h>
#include <udjat/ui/animation.h>
#include <udjat/tools/intl.h>
#include <udjat/tools/logger.h>
#include <cstring>
#include <sstream>

using namespace std;

namespace Udjat {

	// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

	Console::Progress::Progress(const char *title) : std::string{title}, url_text{title} {
		write("\r");
		write(ClearEOL);
		write(CursorInvisible);
		present();
	}

	Console::Progress::~Progress() {
		write("\r");
		write(EraseLine);
		write(CursorVisible);
	}

    Dialog::Progress & Console::Progress::url(const char *u) noexcept {
		url_text = u;
		return *this;
	}

	Dialog::Progress & Console::Progress::set(uint64_t current, uint64_t total, bool) noexcept {
		this->current = current;
		this->total = total;
		present();
		return *this;
	}

    Dialog::Progress & Console::Progress::set(const Console::Color color) noexcept {
		this->color = color;
		present();
		return *this;
	}

    void Console::Progress::present() {

		stringstream buffer;

		int width = (int) Screen::width();
		size_t column = 0;	///< @brief The current screen column; not the same of buffer length.

		buffer << "\r" << EraseLine;

		if(color && *color) {
			buffer << color;
		}

		// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

		// 000000000011111111112222222222333333333344444444445555555555666666666677777777778
		// 012345678901234567890123456789012345678901234567890123456789012345678901234567890
		// * URL.................................. [################################] 100.0%

		buffer << animation.get();
		column++;

		if(width >= 40) {

			buffer << " ";
			column++;

			// Show URL
			{
				string line{url_text};
				size_t szline = line.size();
				size_t window = ((width/2)-column);

				if(window < szline) {
					
					line.resize(window-3);
					line.append("...");
					
				} else if(window > szline) {

					line.resize(window,' ');

				}

				buffer << line;
				column += window;

			}

			buffer << " ";
			column++;

			// Show percent
			{
				size_t szline = (width-(column+8));
				char line[szline+1];
#ifdef DEBUG
				memset(line,'-',szline);
#endif // DEBUG
				line[0] = '[';
				line[szline-1] = ']';
				line[szline] = 0;

				char text[10];
				memset(text,0,10);

				if(!(current||total)) {
					memcpy(text,"0.0%",4);
				} else if(current >= total) {
					memcpy(text,"100.0%",6);
					memset(line+1,'#',szline-2);
				} else {
					float progress = (float) current / (float) total;
					snprintf(text,10,"%3.1f%%",progress * 100.0);
					size_t p = (size_t) ( (((float) szline)-2) * progress);
					memset(line+1,'#',p);
				}

				buffer << line;
				column += szline;

				{
					int spc = 7-strlen(text);
					if(spc > 0) {
						string spaces;
						spaces.resize(spc,' ');
						buffer << spaces;
						column += spc;
					}
				}
				
				buffer << text;
				column += strlen(text);
				
			}

		}

		buffer << '\r';

		Console::write(buffer.str().c_str());

	}

}
