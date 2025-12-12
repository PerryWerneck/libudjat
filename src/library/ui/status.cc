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
  * @brief Implements status dialog.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/status.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>
 #include <private/logger.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif

 using namespace std;

 namespace Udjat {

	Dialog::Status *Dialog::Status::instance = nullptr;

	Dialog::Status::Status() {
		instance = this;
	}

	Dialog::Status::~Status() {
		if(instance == this) {
			instance = nullptr;
		}
	}

	Dialog::Status &Dialog::Status::getConsole() {

		class Status : public Dialog::Status {
		public:
			Status() : Dialog::Status() {}
			~Status() override {}

			Status & state(const char *text) noexcept override {
				Logger::String{text}.write((Logger::Level) (Logger::Debug+1),"state");
#ifndef _WIN32
				if(Logger::decorated()) {
					Logger::write(1,String{"\x1B]0;",text,"\x07"}.c_str());
					fsync(1);
				}
#endif // !_WIN32
				return *this;
			}
		};

		static Status default_instance;
		return default_instance;

	}
	
	Dialog::Status &Dialog::Status::getInstance() {

		if(instance) {
			return *instance;
		}

		return getConsole();

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

	Dialog::Status & Dialog::Status::busy(bool) noexcept {
		return *this;
	}

	Dialog::Status & Dialog::Status::busy(const char *text) noexcept {
		if(text && *text) {
			state(text);
		}
		return busy((text && *text));
	}

}



