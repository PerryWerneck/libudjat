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

 namespace Udjat {

	namespace Alert {

		class Controller;
		class Activation;

	}

	/// @brief Start alert activation.
	UDJAT_API void start(std::shared_ptr<Udjat::Alert::Activation> activation);

	/// @brief Create an alert from XML description;
	/// @param parent Parent object, usually an agent, scanned for alert attributes.
	/// @param node XML description of the alert.
	/// @param type Alert type ('url' or 'script' for internal ones, factory name for module based alerts).
	/// @return Pointer to the new alert.
	UDJAT_API std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const pugi::xml_node &node, const char *type = "default");


 }
