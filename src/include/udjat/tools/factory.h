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
 #include <udjat/agent/abstract.h>

 namespace Udjat {

	/// @brief Object factory.
	/// Create objects from XML definition.
	class UDJAT_API Factory : public Abstract::Agent::Factory {
	private:
		class Controller;

		const char *factory_name = "";

		/// @brief Factory module info.
		const ModuleInfo &module;

	public:
		Factory(const char *name, const ModuleInfo &module);
		virtual ~Factory();

		virtual int compare(const char *name) const noexcept;
		virtual int compare(const XML::Node &node) const noexcept;

		inline bool operator==(const char *name) const noexcept {
			return compare(name) == 0;
		}

		inline bool operator==(const XML::Node &node) const noexcept {
			return compare(node) == 0;
		}

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

		inline const char * name() const {
			return factory_name;
		}

		virtual Value & getProperties(Value &properties) const;

		/// @brief Execute function in all registered factories until it returns true.
		/// @param func	Function to execute.
		/// @return false if the function doesnt returned true for any element.
		static bool for_each(const std::function<bool(Factory &factory)> &func);

		/// @brief Execute function in all factories with 'name' until it returns true.
		/// @param func	Function to execute.
		/// @return false if the function doesnt returned true for any element.
		static bool for_each(const char *name, const std::function<bool(Factory &factory)> &func);

		/// @brief Execute function in all factories who suports the node until it returns true.
		/// @param node The XML node to check.
		/// @param func	Function to execute.
		/// @return false if the function doesnt returned true for any element.
		static bool for_each(const XML::Node &node, const std::function<bool(Factory &factory)> &func);

		/// @brief Create an agent from XML node.
		/// @param node XML definition for the new agent.
		virtual std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object &parent, const XML::Node &node) const override;

		/// @brief Create a child object from XML node.
		/// @param node XML definition for the new state.
		virtual std::shared_ptr<Abstract::Object> ObjectFactory(const Abstract::Object &parent, const XML::Node &node);

		UDJAT_DEPRECATED(virtual std::shared_ptr<Abstract::Object> ObjectFactory(const Abstract::Object &parent, const XML::Node &node) const);

		/// @brief Create an alert from XML node.
		/// @param node XML definition for the new alert.
		virtual std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const XML::Node &node) const;

		/// @brief Create an activatable from XML node.
		/// @param node XML definition for the new alert.
		virtual std::shared_ptr<Activatable> ActivatableFactory(const Abstract::Object &parent, const XML::Node &node) const;

		/// @brief Create an element from xml node (called on any node with attribute type = 'factory_name').
		virtual bool CustomFactory(const XML::Node &node);
		virtual bool CustomFactory(Abstract::Object &parent, const XML::Node &node);

		/// @brief Create a custom element from xml (called on any node <factory_name>).
		virtual bool NodeFactory(const XML::Node &node);
		virtual bool NodeFactory(Abstract::Object &parent, const XML::Node &node);

		/// @brief Parse a generic XML node.
		/// @param XML definition for the new element.
		/// @return true if the node was parsed.
		UDJAT_DEPRECATED(virtual bool generic(const XML::Node &node));

		/// @brief Parse a XML node.
		/// @param object Parent object.
		/// @param XML definition for the new element.
		/// @return true if the node was inserted.
		UDJAT_DEPRECATED(virtual bool generic(Abstract::Object &parent, const XML::Node &node));

	};

 }

