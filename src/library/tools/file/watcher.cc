/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements file watcher.
  */

 #include <config.h>
 #include <udjat/tools/file/watcher.h>
 #include <private/filewatcher.h>
 #include <udjat/tools/object.h>

 namespace Udjat {

	File::Watcher::Watcher(const char *p) : pathname{p} {
	}

	File::Watcher::Watcher(const XML::Node &node, const char *attrname)
		: File::Watcher{Object::getAttribute(node, attrname, "")} {

		if(node.attribute("watch-file-changes").as_bool(true)) {
			watch();
		}

	}

	File::Watcher::~Watcher() {
		unwatch();
	}

	void File::Watcher::updated(const Event, const char *) {
	}

	File::Watcher & File::Watcher::watch() {
		Controller::getInstance().remove(this); // Close watch handlers (Just in case).
		Controller::getInstance().insert(this);	// Open watch handlers.
		return *this;
	}

	File::Watcher & File::Watcher::unwatch() {
		Controller::getInstance().remove(this);	// Close watch handlers.
		return *this;
	}

 }

