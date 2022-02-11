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
 #include <udjat/request.h>
 #include <udjat/tools/object.h>

 namespace Udjat {

	/// @brief Object factory.
	/// Create objects from XML definition.
	class UDJAT_API Factory {
	private:
		class Controller;

		const char *factory_name = "";

		/// @brief Factory module info.
		const ModuleInfo &module;

	public:
		Factory(const char *name, const ModuleInfo &module);
		virtual ~Factory();

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

		inline const char * getName() const {
			return factory_name;
		}

		inline const char * name() const {
			return factory_name;
		}

		/// @brief Find factory by name.
		/// @param name Factory name.
		/// @return The factory with the requested name or nullptr if not found.
		static const Factory * find(const char *name);

		static void getInfo(Response &response);

		/// @brief Search factory for xml defined element.
		/// @param node XML node to start searching for.
		/// @param call Lamba call to test for valid factory.
		/// @param typeattribute The name of the optional attribute with the factory name.
		/// @return true if the lambda has returned true.
		static bool search(const pugi::xml_node &node, const std::function<bool(const Factory &, const pugi::xml_node &)> &call, const char *typeattribute = "type");

		/// @brief Execute function in all registered factories until it returns true.
		/// @param func	Function to execute.
		/// @return false if the function doesnt returned true for any element.
		static bool for_each(std::function<bool(const Factory &factory)> func);

		/// @brief Execute function in all registered factories until it returns true.
		/// @param name	Requested factory name.
		/// @param func	Function to execute.
		/// @return false if the function doesnt returned true for any element.
		static bool for_each(const char *name, std::function<bool(const Factory &factory)> func);

		/// @brief Create an agent from XML node.
		/// @param node XML definition for the new agent.
		virtual std::shared_ptr<Abstract::Agent> AgentFactory(const pugi::xml_node &node) const;

		/// @brief Create a child object from XML node.
		/// @param node XML definition for the new state.
		virtual std::shared_ptr<Abstract::Object> ObjectFactory(const Abstract::Object &parent, const pugi::xml_node &node) const;

		/// @brief Create an alert from XML node.
		/// @param node XML definition for the new alert.
		virtual std::shared_ptr<Abstract::Alert> AlertFactory(const pugi::xml_node &node) const;

		/// @brief Parse agent sub-node.
		/// @param parent Parent agent to insert the built child.
		/// @param node XML definition for the new agent.
		/// @return true if the request was handled.
		virtual bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const;

		/// @brief Parse State sub-node.
		/// @param parent Parent state insert the built child.
		/// @param node XML definition for the new state.
		/// @return true if the request was handled.
		virtual bool parse(Abstract::State &parent, const pugi::xml_node &node) const;

	};

 }

