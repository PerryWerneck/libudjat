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
#include <udjat/ui/animation.h>
#include <memory>

using namespace std;

namespace Udjat {

	Console::Animation::Style Console::Animation::style = Console::Animation::Style::Default;

	void Console::Animation::set(Console::Animation::Style st) {
		if(st >= Console::Animation::Style::Default) {
			st = Console::Animation::Style::Default;
		}
		style = st;	
	}

	const char ** Console::Animation::get_model(Animation::Style style) {
 
		static const char *plaintext[] = { "|", "/", "-", "\\", nullptr }; 
		static const char *simple[] = { "◴","◷","◶","◵", nullptr };
		static const char *braille[] = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏", nullptr };
		static const char *circle[] = { "◜", "◝", "◞", "◟", nullptr };

		if(!Console::decorated()) {
			return plaintext;
		}

		switch(style) {
		case Console::Animation::Style::Default:
		case Console::Animation::Style::Braille:
			return braille;
		
		case Console::Animation::Style::PlainText:
			return plaintext;
		
		case Console::Animation::Style::Simple:
			return simple;
		
		case Console::Animation::Style::Circle:
			return circle;

		}

		throw logic_error("Unexpected of invalid animation type");
	}

	Console::Animation::Animation(Console::Animation::Style style) : model{get_model(style)} {
	}

	const char * Console::Animation::get() noexcept {
		if(!model[current]) {
			current = 0;
		}
		return model[current++];
	}


}
