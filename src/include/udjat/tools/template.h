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
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/file/path.h>
 #include <ostream>
 #include <functional>

 namespace Udjat {

	/// @brief HTTP Template page.
	class UDJAT_API Template {
	private:
		File::Path filepath;

	protected:
		const char *marker = "%{";

	public:

		/// @brief Build a template from properties.
		Template(const Udjat::Properties &props);

		/// @brief Build a template page for mimetyppe.
		Template(const char *name, const MimeType mimetype = MimeType::none);

		inline operator bool() const noexcept {
			return filepath.regular();
		}

		/// @brief Get timestamp of the last file change.
		inline time_t last_modified() const {
			return filepath.last_modified();
		}

		/// @brief Apply template.
		/// @param stream Output stream.
		/// @param callback callback for %{} processing, return true if key was recognized, false if not.
		/// @return string with processed template
		std::string to_string(const std::function<bool(const char *key, std::ostream &stream)> &callback);

		/// @brief Apply template.
		/// @param stream Output stream.
		/// @param callback callback for %{} processing, return true if key was recognized, false if not.
		void apply(std::ostream &stream, const std::function<bool(const char *key, std::ostream &stream)> &callback);

		/// @brief Apply value on template.
		/// @param stream Output stream.
		/// @param value Values for template.
		void apply(std::ostream &stream, const Variant &value);

		/// @brief Apply value on template.
		/// @param stream Output stream.
		/// @param statu HTTP status to apply.
		void apply(std::ostream &stream, const HTTP::Status &status);

	};

 }
