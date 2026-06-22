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
 #include <udjat/ui/menu.h>
 
 namespace Udjat {

	namespace Console {

		/// @brief Get a string based console menu.
		/// @param title Title for the menu.
		/// @return Pointer to abstract menu
		UDJAT_API std::shared_ptr<Udjat::Menu<::std::string>> MenuFactory(const char *title);
       
        size_t UDJAT_API select(const Udjat::Abstract::Menu &menu);

        template <class T>
	    class UDJAT_API Menu : public Udjat::Menu<T> {
        public:
            Menu(const char *t) : Udjat::Menu<T>{t} {			
            }

		    size_t select() const override {
                return Udjat::Console::select(*this);
            }

        };

    }
		
 }


