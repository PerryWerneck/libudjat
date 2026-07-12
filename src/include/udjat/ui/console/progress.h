/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declare application status dialog.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/ui/console.h>
 #include <udjat/ui/progress.h>
 #include <udjat/ui/animation.h>
 #include <string>
 
 namespace Udjat {

	namespace Console {

		/// @brief Get a string based console menu.
		/// @param title Title for the menu.
		/// @return Pointer to abstract menu
		UDJAT_API std::shared_ptr<Udjat::Dialog::Progress> ProgressFactory(const char *title);
  
        class UDJAT_API Progress : public Dialog::Progress, public std::string {
        private:
            Console::Animation animation;
            Console::Color color = nullptr;   ///< @brief Console setup string.
            uint64_t current = 0;
            uint64_t total = 0;

        protected:
            std::string url_text;

            /// @brief Present progress bar at current console line.
            void present();

        public:
            Progress(const char *title = "");
            ~Progress() override;

            Dialog::Progress & set(Console::Color color) noexcept;

			Dialog::Progress & set(uint64_t current, uint64_t total, bool is_file_size = true) noexcept override;
            Dialog::Progress & url(const char *url) noexcept override;
    
        };
  
    }
		
 }


