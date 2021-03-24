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

#include <udjat/defs.h>
#include <udjat/tools/quark.h>

namespace Udjat {

	/// @brief Generic text file object (Don't use for large files).
	class UDJAT_API File {
	private:
		class Controller;
		friend class Controller;

		/// @brief Path to file.
		Quark name;

	protected:

		/// @brief Load file and call loaded() with the contents.
		void load();

		/// @brief Called when file is loaded by call to load() or content changed.
		/// @param The file contents.
		virtual void loaded(const char *contents);

	public:
		File(const char *name);
		File(const Quark &name);
		File(const pugi::xml_node &node);
		File(const pugi::xml_attribute &attribute);

		virtual ~File();


	};

}
