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
#include <private/misc.h>
#include <cstring>
#include <udjat/ui/console.h>
#include <udjat/tools/logger.h>
#include <cstdio>
#include <sys/ioctl.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif 

using namespace std;

namespace Udjat {

	class UDJAT_PRIVATE ConsoleWriter : public std::basic_streambuf<char, std::char_traits<char> > {
	protected:

		/// @brief Writes characters to the associated file from the put area
		int sync() override {
			return 0;
		}

		/// @brief Writes characters to the associated output sequence from the put area.
		int overflow(int c) override {

			if(c && c != EOF) {
				char chr = (char) c;
				if(write(STDOUT_FILENO,&chr,1) != 1) {
					return EOF;
				}
			}

			return c;
		}

	public:
		ConsoleWriter() {
		}

		virtual ~ConsoleWriter() {			
		}

	};

	UI::Console::Console() : enabled{Logger::console()} {
		debug("Console was build");
		static ConsoleWriter writer;
		this->rdbuf(&writer);
		Logger::console(false);
		cursor(false);
	}

	UI::Console::~Console() {
		*this << "\x1B[0m";
		cursor(true);
		Logger::console(enabled);
		debug("Console was deleted");
	}

	unsigned short UI::Console::width() const noexcept {
#ifdef _WIN32

#else
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		return w.ws_col;
#endif // _WIN32
	}

	bool UI::Console::progress(const char *prefix, const char *url, uint64_t current, uint64_t total) noexcept {

		unsigned short width = this->width();
		
		char line[width+1];
		memset(line,' ',width+1);
		line[width] = 0;	

		// 000000000011111111112222222222333333333344444444445555555555666666666677777777778
		// 012345678901234567890123456789012345678901234567890123456789012345678901234567890
		// URL.................................... [################################] 100.0%

		size_t plen = strlen(prefix ? prefix : "");
		size_t ulen = strlen(url);

		if(width >= 40) {

			size_t pos = (width/2);
			{
				memcpy(line, prefix, plen);
				
				plen++;
				int spc = (pos - plen);
				if(spc > (int) ulen) {
					memcpy(line+plen, url, ulen);
				} else {
					memcpy(line+plen,"...",3);
					spc -= 4;
					memcpy(line+plen+3, url+(ulen-spc), spc);
				}

			}

			line[pos++] = '[';
			float progress_len = (float) width - (pos + 7);

			char text[10];
			memset(text,0,10);

			if(total) {

				float progress = (float) current / (float) total;
				if(current >= total) {
					snprintf(text,10,"100.0%%");
				} else {
					snprintf(text,10,"%3.1f%%",progress * 100.0);
				}
		
				size_t p = (size_t) (progress_len * progress);
				memset(line+pos,'#',p);

			} else {

				snprintf(text,10,"0.0%%");
			}

			memcpy(line+width-strlen(text),text,strlen(text));

			pos += (progress_len-1);
			line[pos++] = ']';

		} else {

			if(ulen >= (size_t) (width-8)) {
				strncpy(line,url,width-12);
				memcpy(line+(width-12),"... ",4);
			} else {
				memcpy(line,url,ulen);
			}

			char text[10];
			if(current >= total) {
				snprintf(text,10,"100.0%%");
			} else {
				float progress = (float) current / (float) total;
				snprintf(text,10,"%3.1f%%",progress * 100.0);
			}

			memcpy(line+width-strlen(text),text,strlen(text));

		}

		*this << "\x1B[0G" << line << "\r";

		return false;

	}

	UI::Console & UI::Console::set(const Foreground color) {
		*this << "\x1B[" << (int) color << "m";
		return *this;
	}

	void UI::Console::bold(bool on) {
		*this << "\x1B[" << (on ? "1" : "22") << "m";
	}

	void UI::Console::faint(bool on) {
		*this << "\x1B[" << (on ? "2" : "22") << "m";
	}

	void UI::Console::italic(bool on) {
		*this << "\x1B[" << (on ? "3" : "23") << "m";
	}

	void UI::Console::cursor(bool on) {
		*this << "\x1B[" << (on ? "?25h" : "?25l");
	}

	void UI::Console::up(size_t lines) {
		*this << "\x1B[" << lines << "F";
	}

	void UI::Console::down(size_t lines) {
		*this << "\x1B[" << lines << "E";
	}

}
