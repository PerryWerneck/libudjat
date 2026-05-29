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
#include <cstring>
#include <udjat/ui/console.h>
#include <memory>

using namespace std;

namespace Udjat {

	UI::Animation::Style UI::Animation::style = UI::Animation::Style::Default;

	void UI::Animation::set(UI::Animation::Style st) {
		if(st >= UI::Animation::Style::Default) {
			st = UI::Animation::Style::Default;
		}
		style = st;	
	}

	std::shared_ptr<UI::Animation> UI::Animation::Factory(UI::Animation::Style style) {

		class PlainTextAnimation : public UI::Animation {
		private:
			const char *animation[4] = { "|", "/", "-", "\\" };

		public:
			constexpr PlainTextAnimation() = default;

			const char * get() noexcept override {
				if(current >= sizeof(animation)/sizeof(animation[0])) {
					current = 0;
				}			
				return animation[current++];
			}	

		};

		class SimpleAnimation : public UI::Animation {
		private:
			const char *animation[4] = { "◴","◷","◶","◵" };

		public:
			constexpr SimpleAnimation() = default;

			const char * get() noexcept override {
				if(current >= sizeof(animation)/sizeof(animation[0])) {
					current = 0;
				}			
				return animation[current++];
			}	

		};

		class BrailleAnimation : public UI::Animation {
		private:
			const char *animation[10] = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" };

		public:
			constexpr BrailleAnimation() = default;

			const char * get() noexcept override {
				if(current >= sizeof(animation)/sizeof(animation[0])) {
					current = 0;
				}			
				return animation[current++];
			}	

		};

		class CircleAnimation : public UI::Animation {
		private:
			const char *animation[4] = { "◜", "◝", "◞" , "◟" };

		public:
			constexpr CircleAnimation() = default;

			const char * get() noexcept override {
				if(current >= sizeof(animation)/sizeof(animation[0])) {
					current = 0;
				}			
				return animation[current++];
			}	

		};

		if(style == UI::Animation::Style::Default) {
			style = UI::Animation::style;
		}

		switch(style) {
		case UI::Animation::Style::Default:
			break;

		case UI::Animation::Style::PlainText:
			return make_shared<PlainTextAnimation>();
		
		case UI::Animation::Style::Simple:
			return make_shared<SimpleAnimation>();
		
		case UI::Animation::Style::Braille:
			return make_shared<BrailleAnimation>();
		
		case UI::Animation::Style::Circle:
			return make_shared<CircleAnimation>();
		}

#ifdef _WIN32
		return make_shared<PlainTextAnimation>();
#else
		return make_shared<BrailleAnimation>();
#endif

	}

}
