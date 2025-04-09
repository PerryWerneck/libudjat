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
 #include <udjat/tools/logger.h>
 #include <private/dialog.h>
 #include <memory>
 #include <mutex>

 using namespace std;

 namespace Udjat {

	std::mutex UI::Dialog::guard;
	std::shared_ptr<UI::Console> UI::Dialog::sptr;

	UI::Dialog::Dialog() {
		lock_guard<mutex> lock(guard);

		if(!sptr) {
			sptr = std::make_shared<Console>();
		}
	
		console = sptr;

		debug("Dialog was built");

	}

	UI::Dialog::~Dialog() {

		lock_guard<mutex> lock(guard);
		debug("Dialog was deleted, use_count=",sptr.use_count());
		if(sptr.use_count() == 2) {
			sptr.reset();
		}

	}

 }

