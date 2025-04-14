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
 #include <string>

 namespace Udjat {

	namespace Dialog {

		/// @brief Progress bar.
		/// @details Abstract progress bar, the implementation depends on the application.
		class UDJAT_API Progress {
		protected:
			Progress();
		  
		public:

			class UDJAT_API Factory {
			private:
				static Factory *instance;
				Factory *parent;
			
			public:

				static Factory & getInstance();

				Factory();
				virtual ~Factory();
				virtual std::shared_ptr<Progress> ProgressFactory() const = 0;

			};

			Progress(const Progress &other) = delete;
			Progress(Progress *other) = delete;

			/// @brief Get a new instance of dialog progress.
			/// @return The dialog progress to use (if it's a new one or an already built depends on the factory).
			static std::shared_ptr<Progress> getInstance();

			virtual ~Progress();

			virtual Progress & title(const char *title);
			virtual Progress & item(const short current = 0, const short total = 0);
			virtual Progress & set(uint64_t current = 0, uint64_t total = 0, bool is_file_size = true);
	
			virtual Progress & show();
			virtual Progress & hide();

			/// @brief Set progress bar URL.
			virtual Progress & set(const char *url);
		
			inline Progress & set(const std::string &s) {
				return set(s.c_str());
			}

			inline Progress & operator = (const char *u) {
				return set(u);
			}

			inline Progress & operator = (const std::string &s) {
				return set(s.c_str());
			}
			
		};

	}

 }


