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
 #include <udjat/tools/object.h>
 #include <memory>

 namespace Udjat {

	class UDJAT_API Activatable : public NamedObject {
	public:

		constexpr Activatable(const char *name = "") : NamedObject{name} {
		}

		Activatable(const XML::Node &node) : NamedObject{node} {
		}

		static std::shared_ptr<Activatable> Factory(const Abstract::Object &parent, const XML::Node &node);

		/// @brief Activate object, apply values.
		virtual void activate(const std::function<bool(const char *key, std::string &value)> &expander) = 0;

		/// @brief Activate object, expand properties.
		void activate(const Abstract::Object &object);

		virtual void deactivate();

		/// @brief Is the object activated?
		virtual bool activated() const noexcept = 0;

		/// @brief Trigger (deactivate/activate).
		void trigger(const Abstract::Object &object);


	};

 }
