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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/event.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	mutex Event::guard;

	Event::Event() {
	}

	Event::~Event() {
	}

	std::string Event::to_string() const noexcept {
		return "event";
	}

	void Event::trigger() noexcept {

		std::lock_guard<std::mutex> lock(guard);
		string name = to_string();

		listeners.remove_if([name](const Listener &listener){

			try {
				if(listener.handler()) {
					return false;
				}
			} catch(const std::exception &e) {
				cerr << name << "\t" << e.what() << endl;
			} catch(...) {
				cerr << name << "\tUnexpected error processing event" << endl;
			}

			return true;

		});


	}

	void Event::insert(void *id, const std::function<bool()> handler) {
		std::lock_guard<std::mutex> lock(guard);
		listeners.emplace_front(id,handler);
	}

	void Event::remove(void *id) {
		std::lock_guard<std::mutex> lock(guard);
		listeners.remove_if([id](const Listener &listener){
			return listener.id == id;
		});
	}

 }
