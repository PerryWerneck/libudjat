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

using namespace std;

namespace Udjat {

	Console::Progress::Progress(const char *title) : std::string{title}, url_text{title} {
		Console::write("\x1b[?25l");

	}

	Console::Progress::~Progress() {
		Console::write("\r\x1b[2K\x1b[?25h");
	}

    Dialog::Progress & Console::Progress::url(const char *u) noexcept {
		url_text = u;
		return *this;
	}

	Dialog::Progress & Console::Progress::set(uint64_t current, uint64_t total, bool is_file_size) noexcept {

		int width = (int) Screen::width();

		size_t len = (width*4);
		char buffer[len+1];
		char *dst = buffer;

		memset(buffer,' ',len);
		buffer[len] = 0; // Just in case.

		strcpy(dst,"\r\x1b[2K");
		dst += strlen(dst);

		int pos = 0;

		// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

		// 000000000011111111112222222222333333333344444444445555555555666666666677777777778
		// 012345678901234567890123456789012345678901234567890123456789012345678901234567890
		// * URL.................................. [################################] 100.0%

		{
			const char *str = animation.get();
			size_t slen = strlen(str);
			strncpy(dst,str,slen);
			dst += slen;
			pos++;
		}

		if(width >= 40) {

			{
				*(dst++) = ' ';
				pos++;
			}

			// Show URL
			{
				string line{url_text};
				size_t szline = line.size();
				size_t window = ((width/2)-pos);

				if(window < szline) {
					
					line.resize(window-3);
					line.append("...");
					
				} else if(window > szline) {

					line.resize(window,' ');

				}

				memcpy(dst,line.c_str(),window);
				dst += window;
				pos += window; 

			}

			{
				*(dst++) = ' ';
				pos++;
			}

			// Show percent
			{
				size_t szline = (width-(pos+8));
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

				memcpy(dst,line,szline);
				dst += szline;
				pos += szline;

				{
					size_t spc = 7-strlen(text);
					dst += spc;
					pos += spc;
				}
				
				memcpy(dst,text,strlen(text));
				dst += strlen(text);
				pos += strlen(text);
				
			}

		}


		// Write to console.
		*(dst++) = '\r';
		*dst = 0;
		Console::write(buffer);

		return *this;
	}

}
