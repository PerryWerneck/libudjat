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
#include <list>
#include <algorithm>
#include <string>

namespace Udjat {

	namespace File {

		/// @brief Directory contents.
		class UDJAT_API List : public std::list<std::string> {
		public:
			List(const char *pattern);
			~List();
			void forEach(std::function<void (const char *filename)> call);
		};

		/// @brief Text file agent.
		///
		/// Monitor a local file and call 'load' method when it changes.
		///
		class UDJAT_API Agent {
		private:
			class Controller;
			friend class Controller;

			/// @brief Path to file.
			Quark name;

		protected:

			/// @brief Called when the file changes.
			/// @param The file contents.
			virtual void set(const char *contents);

		public:
			Agent(const char *name);
			Agent(const Quark &name);
			Agent(const pugi::xml_node &node);
			Agent(const pugi::xml_attribute &attribute);

			inline const char * getName() const {
				return name.c_str();
			}

			virtual ~Agent();

		};

		/// @brief Generic text file object (Don't use for large files).
		class UDJAT_API Local {
		private:
			void * contents;	///< @brief File contents.
			size_t length;		///< @brief File length.

		public:
			Local(const char *filename);
			Local(const File::Agent &agent);
			~Local();

			inline size_t size() const noexcept {
				return this->length;
			}

			inline const char * c_str() const noexcept {
				return (const char *) contents;
			}

		};

	}

}
