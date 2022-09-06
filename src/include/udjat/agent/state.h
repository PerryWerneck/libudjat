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

/**
 * @file src/include/udjat/agent/state.h
 *
 * @brief Declare the agent states.
 *
 * @author perry.werneck@gmail.com
 *
 */

#pragma once

#include <string>
#include <pugixml.hpp>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>
#include <udjat/defs.h>
#include <udjat/tools/quark.h>
#include <udjat/tools/xml.h>
#include <udjat/request.h>
#include <udjat/alert/abstract.h>
#include <udjat/tools/value.h>
#include <udjat/tools/object.h>
#include <udjat/tools/parse.h>
#include <cstring>
#include <ostream>
#include <udjat/agent/level.h>

namespace Udjat {

	/// @brief Get OStream from level.
	UDJAT_API std::ostream & LogFactory(Udjat::Level level);

	namespace Abstract {

		class UDJAT_API State : public Udjat::Object {
		public:

			/// @brief Notify on state activation?
			bool notify = false;

		private:

			/// @brief Parse XML node
			void set(const pugi::xml_node &node);

			/// @brief State alerts.
			std::vector<std::shared_ptr<Abstract::Alert>> alerts;

		protected:

			struct Properties {

				/// @brief State level.
				Level level = unimportant;

				/// @brief Message body.
				const char * body = "";

			} properties;

		public:

			/// @brief Create state using the strings without conversion.
			State(const char *name, const Level level = Level::unimportant, const char *summary = "", const char *body = "");

			/// @brief Create state from xml node
			State(const pugi::xml_node &node);

			std::string to_string() const override {
				return Object::properties.summary;
			}

			virtual ~State();

			inline const char * body() const noexcept {
				return properties.body;
			}

			/// @brief Get the state level.
			/// @return The state level.
			/// @see Level
			inline Level level() const noexcept {
				return properties.level;
			}

			inline bool operator ==(const Level level) const noexcept {
				return properties.level == level;
			}

			/// @brief Is this state a critical one?
			/// @return true if the state is critical.
			inline bool critical() const noexcept {
				return level() >= Level::critical;
			}

			/// @brief Check if this state is a problem.
			/// @return true if the state is not a problem.
			inline bool ready() const noexcept {
				return level() <= Level::ready;
			}

			virtual void get(const Request &request, Response &response) const;

			virtual void activate(const Agent &agent) noexcept;
			virtual void deactivate(const Agent &agent) noexcept;

			/// @brief Get property.
			/// @param key The property name.
			/// @param value String to update with the property value.
			/// @return true if the property is valid.
			bool getProperty(const char *key, std::string &value) const noexcept override;

			/// @brief Get the state properties.
			/// @brief Value to receive the properties.
			/// @return Pointer to value.
			Value & getProperties(Value &value) const noexcept override;

			/// @brief Insert alert.
			inline void push_back(std::shared_ptr<Abstract::Alert> alert) {
				alerts.push_back(alert);
			}

			/// @brief Create and insert child.
			/// @param type The agent type.
			/// @param node XML agent definitions.
			/// @return true if the child was created.
			bool push_back(const char *type, const pugi::xml_node &node);

			/// @brief Create and insert child from XML definition.
			/// @param node XML agent definitions.
			/// @return true if the child was created.
			bool push_back(const pugi::xml_node &node);

		};

	}

	template <typename T>
	class UDJAT_API State : public Abstract::State {
	protected:

		/// @brief Minimum value for state activation.
		T from;

		/// @brief Maximum value for state activation.
		T to;

	public:
		State(const char *name, const T f, const T t, const Level level, const char *summary = "", const char *body = "")
				: Abstract::State(name,level,summary,body), from(f),to(t) { }

		State(const char *name, const T value, const Level level, const char *summary = "", const char *body = "")
				: Abstract::State(name,level,summary,body), from(value),to(value) { }

		State(const pugi::xml_node &node) : Abstract::State(node) {
			XML::parse(node,from,to);
		}

		bool compare(T value) {
			return value >= from && value <= to;
		}

	};


	template <>
	class UDJAT_API State<std::string> : public Abstract::State {
	private:

		/// @brief State value;
		std::string value;

	public:
		State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::Attribute(node,"value",false).as_string()) {
		}

		bool compare(const std::string &value) {
			return strcasecmp(this->value.c_str(),value.c_str()) == 0;
		}

	};

	template <>
	class UDJAT_API State<bool> : public Abstract::State {
	private:

		/// @brief State value;
		bool value;

	public:
		State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::Attribute(node,"value",false).as_bool()) {
		}

		bool compare(const bool value) {
			return this->value == value;
		}

	};

	/// @brief Create an state from exception.
	/// @param except The exception.
	/// @param summary State summary (for message).
	/// @return A new state object based on the exception type and message.
	UDJAT_API std::shared_ptr<Abstract::State> StateFactory(const std::exception &except, const char *summary);

}
