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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/abstract/object.h>
 #include <memory>

 namespace Udjat {

	class UDJAT_API Activatable {
	private:
		const char *object_name;

	protected:

		typedef Activatable super;

		constexpr Activatable(const char *name = "") : object_name{name} {
		}
	
		Activatable(const XML::Node &node);
		virtual ~Activatable();

		/// @brief Convenience method to get payload from xml
		static const char * payload(const XML::Node &node);

		/// @brief Convenience method to capture and translate exceptions.
		int exec(Udjat::Value &response, bool except, const std::function<int()> &func);

	public:

		inline const char *name() const noexcept {
			return object_name;
		}

		inline const char *c_str() const noexcept {
			return object_name;
		}

		/// @brief Activate/deactivate by parameter.
		/// @param value true to activate, false to deactivate.
		/// @return true if the state was changed.
		bool active(bool value) noexcept;

		/// @brief Activate object.
		/// @return true if the object was activated, false if already active.
		virtual bool activate() noexcept = 0;

		/// @brief Activate object with properties.
		/// @param object Object with properties.
		/// @return true if the object was activated, false if already active.
		virtual bool activate(const Udjat::Abstract::Object &object) noexcept;

		/// @brief Deactivate object.
		/// @return true if the object was deactivated, false if already inactive.
		virtual bool deactivate() noexcept;

	};

 }
