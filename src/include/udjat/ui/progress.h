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
  * @brief Declare Console Writer.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <cstdint>
 #include <cstddef>
 #include <memory>

 namespace Udjat {

	namespace Dialog {

		/// @brief Progress dialog.
		/// @details Abstract progress dialog, the implementation depends on the application.
		class UDJAT_API Progress {
		protected:
			Progress();
		  
		public:

			class Factory {
			private:
				static Factory *instance;
		  
			public:
				Factory();
				virtual ~Factory();
				virtual std::shared_ptr<Progress> ProgressFactory() const = 0;

			};

			static std::shared_ptr<Progress> Factory();

			Progress(const Progress &other) = delete;
			Progress(Progress *other) = delete;

			virtual ~Progress();
		
			virtual Progress & title(const char *text);
			virtual Progress & message(const char *text);
			virtual Progress & body(const char *text);
			virtual Progress & icon_name(const char *name);
			virtual Progress & show();
			virtual Progress & hide();
			virtual Progress & item(const size_t current = 0, const size_t total = 0);
	
			/// @brief Set progress bar URL.
			virtual Progress & url(const char *url);
	
			virtual void set(uint64_t current = 0, uint64_t total = 0);
	
			inline Progress & operator = (const char *text) {
				return body(text);
			}
	
		};

	}

 }


