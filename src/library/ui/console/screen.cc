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
#include <udjat/defs.h>
#include <udjat/ui/console.h>

namespace Udjat {

	UI::Console & Console::Screen::bold(bool on) {
		Console::write(on ? SetBold : ResetBold);
		return *this;
	}

	UI::Console & Console::Screen::faint(bool on) {
		Console::write(on ? SetFaint : ResetFaint);
		return *this;
	}

	UI::Console & Console::Screen::italic(bool on) {
		Console::write(on ? SetItalic : ResetItalic);
		return *this;
	}

	UI::Console & Console::Screen::cursor(bool on) {
		Console::write(on ? CursorVisible : CursorInvisible);
		return *this;
	}

	UI::Console & Console::Screen::erase_line() {
		Console::write(EraseLine);
		*this << "\x1B[2K";
		return *this;
	}

}
