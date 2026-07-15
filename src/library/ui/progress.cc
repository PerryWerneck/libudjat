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

 /**
  * @brief Implements the abstract progress bar.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/progress.h>

 namespace Udjat {

	Dialog::Progress::Progress() {
	}

	Dialog::Progress::~Progress() {
	}

	Dialog::Progress & Dialog::Progress::show() noexcept{
		return *this;
	}

	Dialog::Progress & Dialog::Progress::hide() noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::done(bool) noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::title(const char *) noexcept{
		return *this;
	}

	Dialog::Progress & Dialog::Progress::step(const unsigned int, const unsigned int) noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::set(uint64_t, uint64_t, bool) noexcept {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::url(const char *) noexcept{
		return *this;
	}

	Dialog::Progress & Dialog::Progress::message(const char *) noexcept {
		return *this;
	}

 }
