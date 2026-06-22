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
 #include <memory>
 #include <string>
 #include <vector>
 #include <functional>

 namespace Udjat {

	namespace Abstract {

		class UDJAT_API Menu {
		private:
			std::string title;

		protected:
			size_t lpp = 20;

		public:
			Menu(const char *t) : title{t} {			
			}

			inline Menu & lines_per_page(size_t value) noexcept {
				lpp = value;
				return *this;
			}

			inline size_t lines_per_page(void) const noexcept {
				return lpp;
			}

			inline const char *c_str() const noexcept {
				return title.c_str();
			}

			virtual size_t size() const noexcept = 0;

			/// @brief Select option, return index or throw system_error(ECANCELLED) if user cancel.
			/// @param options The options to select.
			/// @return The index of the selected option.
			virtual size_t select() const = 0;

			virtual const std::string label(size_t ix, bool decorated = false) const = 0;

		};

	}

	/// @brief Simple popup menu with options.
	template <class T>
	class UDJAT_API Menu : public Abstract::Menu, public std::vector<T> {
	public:

		typedef Menu<T> super;

		Menu(const char *t) : Abstract::Menu{t} {			
		}

		template<typename... Targs>
		Menu(const char *title, Targs... Fargs) : Menu{title} {
			append(Fargs...);
		}

		template<typename... Targs>
		inline void append(const T &option, Targs... Fargs) {
			this->push_back(option);
			append(Fargs...);
		}

		inline Menu & append(const T &option) {
			this->push_back(option);
			return *this;
		}

		const std::string label(size_t ix, bool decorated = false) const override;

		size_t size() const noexcept override {
			return std::vector<T>::size();
		}

	};

	template <class T>
    inline const std::string Menu<T>::label(size_t ix, bool decorated) const {
        return std::to_string(this->at(ix)); 
    }

	template <>
	inline const std::string Menu<std::string>::label(size_t ix, bool decorated) const {
		return this->at(ix);
	}


 }


