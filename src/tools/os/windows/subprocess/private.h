/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/win32/handler.h>
 #include <udjat/win32/exception.h>

 using namespace std;

 namespace Udjat {

	class UDJAT_PRIVATE SubProcess::Handler : public Win32::Handler {
	private:
		size_t length = 0;
		char buffer[256];

	protected:

		bool handle(bool abandoned) override;

	public:
		Handler() = default;

		void parse();

		virtual void on_error(const char *reason) = 0;
		virtual void on_input(const char *line) = 0;

	}

 }

