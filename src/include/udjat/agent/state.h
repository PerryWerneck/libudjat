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
 #include <udjat/tools/xml.h>
 #include <memory>
 #include <vector>
 #include <mutex>
 #include <functional>
 #include <iostream>
 #include <udjat/defs.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/parse.h>
 #include <cstring>
 #include <ostream>
 #include <udjat/agent/level.h>
 #include <udjat/tools/converters.h>

 namespace Udjat {

	/// @brief Get OStream from level.
	UDJAT_API std::ostream & LogFactory(Udjat::Level level);

	namespace Abstract {

		class UDJAT_API State : public Udjat::Object {
		public:

			/// @brief Notify on state activation?
			bool notify = false;

		private:

			/// @brief State agents.
			std::vector<std::shared_ptr<Abstract::Agent>> agents;

		protected:

			/// @brief State alerts.
			std::vector<std::shared_ptr<Udjat::Activatable>> listeners;

			struct Properties {

				/// @brief State level.
				Level level = unimportant;

				/// @brief Message body.
				const char * body = "";

			} properties;

			struct {
				bool forward = false;	///< @brief Forward to children?
			} options;

		public:

			State(const State&) = delete;
			State& operator=(const State &) = delete;
			State(State &&) = delete;
			State & operator=(State &&) = delete;

			/// @brief Parse XML node, append child.
			/// @param node The XML definition.
			/// @return true if the child was appended.
			virtual bool parse(const XML::Node &node) override;

			/// @brief Create state using the strings without conversion.
			State(const char *name, const Level level = Level::unimportant, const char *summary = "", const char *body = "");

			/// @brief Create state from xml node
			State(const XML::Node &node);

			/// @brief Get state values as string.
			virtual std::string value() const;

			/// @brief Refresh associated agents.
			void refresh();

			std::string to_string() const noexcept override {
				return Object::properties.summary;
			}

			virtual ~State();

			/// @brief Forward state to children?
			inline bool forward() const noexcept {
				return options.forward;
			}

			inline const char * body() const noexcept {
				return properties.body;
			}

			/// @brief Get the state level.
			/// @return The state level.
			/// @see Level
			inline Level level() const noexcept {
				return properties.level;
			}

#if __cplusplus >= 202002L

			inline int operator <=>(const Level level) const noexcept {
				return properties.level - level;
			}

			inline int operator <=>(const Abstract::State &state) const noexcept {
				return properties.level - state.properties.level;
			}

#else
			inline bool operator ==(const Level level) const noexcept {
				return properties.level == level;
			}

			inline bool operator>(const Abstract::State &state) {
				return properties.level > state.properties.level;
			}

			inline bool operator<(const Abstract::State &state) {
				return properties.level < state.properties.level;
			}

			inline bool operator>=(const Abstract::State &state) {
				return properties.level >= state.properties.level;
			}

			inline bool operator<=(const Abstract::State &state) {
				return properties.level <= state.properties.level;
			}
#endif

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

			virtual void activate(const Abstract::Object &object) noexcept;
			virtual void deactivate() noexcept;

			/// @brief Name of the object icon (https://specifications.freedesktop.org/icon-naming-spec/latest/)
			const char * icon() const noexcept override;

			/// @brief Get property.
			/// @param key The property name.
			/// @param value String to update with the property value.
			/// @return true if the property is valid.
			bool getProperty(const char *key, std::string &value) const override;

			/// @brief Get the state properties.
			/// @brief Value to receive the properties.
			/// @return Pointer to value.
			Value & getProperties(Value &value) const override;

			/// @brief Create an state from exception.
			/// @param except The exception.
			/// @param summary State summary (for message).
			/// @return A new state object based on the exception type and message.
			static std::shared_ptr<Abstract::State> Factory(const std::exception &except, const char *summary);

			/// @brief Create an state.
			static std::shared_ptr<Abstract::State> Factory(const char *name, const Udjat::Level level, int code = 0, const char *summary = "", const char *body = "");

		};

	}

	template <typename T>
	class UDJAT_API State : public Abstract::State {
	protected:

		typedef State<T> super;

		/// @brief Minimum value for state activation.
		T from;

		/// @brief Maximum value for state activation.
		T to;

	public:
		State(const char *name, const T f, const T t, const Level level, const char *summary = "", const char *body = "")
				: Abstract::State(name,level,summary,body), from(f),to(t) { }

		State(const char *name, const T value, const Level level, const char *summary = "", const char *body = "")
				: Abstract::State{name,level,summary,body}, from{value},to{value} { }

		State(const XML::Node &node, const T v = (T) 0) : Abstract::State{node}, from{from_xml<T>(node, v)}, to{from_xml<T>(node, v)} {
			from = from_xml<T>(node,from,"from");
			to = from_xml<T>(node,to,"to");
		}

		State(const XML::Node &node, T from_value, T to_value) : Abstract::State{node}, from{from_xml<T>(node, from_value)}, to{from_xml<T>(node, to_value)} {
		}

		inline bool compare(T value) {
			return value >= from && value <= to;
		}

		inline bool contains(T value) {
			return value >= from && value <= to;
		}

		inline bool operator==(const T value) const noexcept {
			return value == from && from == to;
		}

		inline operator T() const noexcept {
			return from;
		}

		std::string value() const override {

			if(from == to) {
				return std::to_string(from);
			}

			std::string text;
			text += std::to_string(from);
			text += "->";
			text += std::to_string(to);
			return text;
		}

	};


	template <>
	class UDJAT_API State<std::string> : public Abstract::State, public std::string {
	protected:

		typedef State<std::string> super;

	public:
		State(const XML::Node &node) : Abstract::State(node),std::string(Udjat::Attribute(node,"value",false).as_string()) {
		}

		bool compare(const std::string &value) {
			return strcasecmp(this->std::string::c_str(),value.c_str()) == 0;
		}

#if __cplusplus >= 202002L

			inline int operator <=>(const std::string &value) const noexcept {
				return strcasecmp(this->std::string::c_str(),value.c_str());
			}

			inline int operator <=>(const char *value) const noexcept {
				return strcasecmp(this->std::string::c_str(),value);
			}

#else

		inline bool operator==(const std::string &value) {
			return strcasecmp(this->std::string::c_str(),value.c_str()) == 0;
		}

		inline bool operator==(const char *value) {
			return strcasecmp(this->std::string::c_str(),value) == 0;
		}

#endif

		std::string value() const override {
			return *this;
		}

	};

	template <>
	class UDJAT_API State<bool> : public Abstract::State {
	protected:

		typedef State<bool> super;

	private:

		/// @brief The State value.
		bool state_value;

	public:
		State(const XML::Node &node) : Abstract::State(node),state_value(Udjat::Attribute(node,"value",false).as_bool()) {
		}

		bool compare(const bool value) {
			return this->state_value == value;
		}

		inline bool operator==(const bool value) {
			return this->state_value == value;
		}

		std::string value() const override;

	};

 }

 namespace std {

	inline std::string to_string(const Udjat::Abstract::State &state) noexcept {
		return state.to_string();
	}

	inline ostream& operator<< (ostream& os, Udjat::Abstract::State state) noexcept {
			return os << to_string(state);
	}

 }
