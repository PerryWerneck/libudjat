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
 #include <udjat/tools/request.h>
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

		//inline bool operator==(const char *name) const noexcept {
		//	return strcasecmp(name,this->factory_name) == 0;
		//}

		virtual bool probe(const XML::Node &node) const noexcept;

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

		//inline const char * getName() const {
		//	return factory_name;
		//}

		inline const char * name() const {
			return factory_name;
		}

		/// @brief Find factory by name.
		/// @param name Factory name.
		/// @return The factory with the requested name or nullptr if not found.
		// static Factory * find(const char *name);

		virtual Value & getProperties(Value &properties) const;

		/// @brief Search factory for xml defined element.
		/// @param node XML node to start searching for.
		/// @param call Lamba call to test for valid factory.
		/// @param typeattribute The name of the optional attribute with the factory name.
		/// @return true if the lambda has returned true.
		//static bool search(const XML::Node &node, const std::function<bool(Factory &, const XML::Node &)> &call, const char *typeattribute = "type");

		/// @brief Execute function in all registered factories until it returns true.
		/// @param func	Function to execute.
		/// @return false if the function doesnt returned true for any element.
		static bool for_each(const std::function<bool(Factory &factory)> &method);

		/// @brief Execute function in all registered factories until it returns true.
		/// @param name	Requested factory name.
		/// @param func	Function to execute.
		/// @return false if the function doesnt returned true for any element.
		//static bool for_each(const char *name, const std::function<bool(Factory &factory)> &func);

		/// @brief Create an agent from XML node.
		/// @param node XML definition for the new agent.
		virtual std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &parent, const XML::Node &node) const;

		/// @brief Create a child object from XML node.
		/// @param node XML definition for the new state.
		virtual std::shared_ptr<Abstract::Object> ObjectFactory(const Abstract::Object &parent, const XML::Node &node) const;

		/// @brief Create an alert from XML node.
		/// @param node XML definition for the new alert.
		virtual std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const XML::Node &node) const;

		/// @brief Create an activatable from XML node.
		/// @param node XML definition for the new alert.
		virtual std::shared_ptr<Activatable> ActivatableFactory(const Abstract::Object &parent, const XML::Node &node) const;

		/// @brief Parse a generic XML node.
		/// @param XML definition for the new element.
		/// @return true if the node was parset.
		virtual bool generic(const XML::Node &node);

		/// @brief Parse a XML node.
		/// @param object Parent object.
		/// @param XML definition for the new element.
		/// @return true if the node was inserted.
		virtual bool generic(Abstract::Object &parent, const XML::Node &node);

	};

 }

