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
  * @brief Implements the abstract application status.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/status.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Dialog::Status *Dialog::Status::instance = nullptr;

	Dialog::Status::Status() {
		instance = this;
	}

	Dialog::Status::~Status() {
		instance = nullptr;
	}

	Dialog::Status &Dialog::Status::getInstance() {
		if (!instance) {
			throw logic_error("Status instance not initialized");
		}
		return *instance;
	}

	Dialog::Status & Dialog::Status::title(const char *) noexcept {
		return *this;
	}

	Dialog::Status & Dialog::Status::sub_title(const char *) noexcept{
		return *this;
	}

	Dialog::Status & Dialog::Status::state(const Level level, const char *text) noexcept {
		return sub_title(text);
	}

	Dialog::Status & Dialog::Status::state(const char *text) noexcept {
		return state(Level::undefined, text);
	}

	Dialog::Status & Dialog::Status::icon(const char *) noexcept{
		return *this;
	}

	Dialog::Status & Dialog::Status::show() noexcept {
		return *this;
	}

	Dialog::Status & Dialog::Status::hide() noexcept {	
		return *this;
	}

	Dialog::Status & Dialog::Status::step(unsigned int, unsigned int) noexcept{
		return *this;
	}

}



