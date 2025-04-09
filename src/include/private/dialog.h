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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/console.h>
 #include <memory>
 #include <mutex>
 
 namespace Udjat {

    namespace UI {

        class UDJAT_API Dialog {
        public:
            Dialog();
            virtual ~Dialog();
        
        private:
            static std::mutex guard;
            static std::shared_ptr<Console> sptr;

        protected:
            std::shared_ptr<Console> console;

        };
            
    }

 }


