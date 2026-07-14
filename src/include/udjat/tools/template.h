/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Brief description of this source.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/file/path.h>
 #include <ostream>
 #include <functional>

 namespace Udjat {

	/// @brief HTTP Template page.
	class UDJAT_API Template {
	private:
		File::Path filepath;

	public:

		/// @brief Build a template page for mimetyppe.
		Template(const char *name, const MimeType mimetype = MimeType::none);

		inline operator bool() const noexcept {
			return (bool) filepath;
		}

		/// @brief Apply template.
		/// @param code HTTP status code.
		/// @param stream Output stream.
		/// @param name Template name.
		/// @param callback callback for %{} processing, return true if key was recognized, false if not.
		void apply(int code, std::ostream &stream, const std::function<bool(const char *key, std::ostream &stream)> &callback);

	};

 }
