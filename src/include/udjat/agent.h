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
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/converters.h>

 namespace Udjat {

	/// @brief Load XML application definitions.
	/// @param pathname Path to a single xml file or a folder with xml files.
	/// @param force Do a reconfiguration even if the file hasn't change.
	/// @return Seconds for file refresh.
	UDJAT_API time_t reconfigure(const char *pathname, bool force = false);

	/// @brief Load XML application definitions.
	/// @param agent New root agent.
	/// @param pathname Path to a single xml file or a folder with xml files.
	/// @param force Do a reconfiguration even if the file hasn't change.
	/// @return Seconds for file refresh.
	UDJAT_API time_t reconfigure(std::shared_ptr<Abstract::Agent> agent, const char *pathname, bool force = false);

	template <typename T>
	class UDJAT_API Agent : public Abstract::Agent {
	private:

		/// @brief Agent value.
		T value;

		/// @brief Agent states.
		std::vector<std::shared_ptr<State<T>>> statelist;

	protected:

		/// @brief Get agent states.
		inline const std::vector<std::shared_ptr<State<T>>> states() const {
			return this->statelist;
		}

		std::shared_ptr<Abstract::State> stateFromValue() const override {
			for(auto state : statelist) {
				if(state->compare(this->value))
					return state;
			}
			return super::stateFromValue();
		}

		Udjat::Value & get(Udjat::Value &value) const override {
			return value.set(this->value);
		}

		/// @brief Insert state.
		void push_back(std::shared_ptr<State<T>> state) {
			statelist.push_back(state);
		}

	public:

		Agent(const pugi::xml_node &node, const T v = 0) : Abstract::Agent(node), value(v) {
			to_value(node, value);
		}

		Agent(const char *name = "") : Abstract::Agent(name), value(0) {
		}

		Agent(const char *name, const T v) : Abstract::Agent(name), value(v) {
		}

		bool set(const T &value) {

			if(value == this->value)
				return updated(false);

			this->value = value;
			return updated(true);
		}

		bool assign(const char *value) override {
			T new_value;
			to_value(value,new_value);
			return set(new_value);
		}

		inline Agent & operator = (const T value) {
			set(value);
			return *this;
		}

		inline Agent & operator = (const char *value) {
			assign(value);
			return *this;
		}

		T get() const noexcept {
			return value;
		}

		/// @brief Insert State.
		std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override {
			auto state = std::make_shared<State<T>>(node);
			push_back(state);
			return state;
		}

		std::string to_string() const override {
			return std::to_string(value);
		}

	};

	template <>
	class UDJAT_API Agent<std::string> : public Abstract::Agent {
	private:

		/// @brief Agent value.
		std::string value;

		/// @brief Agent states.
		std::vector<std::shared_ptr<State<std::string>>> states;

	protected:

		std::shared_ptr<Abstract::State> stateFromValue() const override {
			for(auto state : states) {
				if(state->compare(this->value))
					return state;
			}
			return super::stateFromValue();
		}

		Udjat::Value & get(Udjat::Value &value) const override {
			return value.set(this->value);
		}

		/// @brief Insert state.
		void push_back(std::shared_ptr<State<std::string>> state) {
			states.push_back(state);
		}

	public:
		Agent(const pugi::xml_node &node) : Abstract::Agent(node), value(node.attribute("value").as_string()) {
		}

		Agent(const char *name = "") : Abstract::Agent(name) {
		}

		Agent(const char *name, const char *v) : Abstract::Agent(name), value(v) {
		}

		bool set(const std::string &value) {

			if(value == this->value)
				return updated(false);

			this->value = value;
			return updated(true);
		}

		std::string get() const noexcept {
			return value;
		}

		bool assign(const char *value) override {

			if(::strcmp(value,this->value.c_str()))
				return updated(false);

			this->value = value;
			return updated(true);

		}

		inline Agent & operator = (const std::string & value) {
			set(value);
			return *this;
		}

		inline Agent & operator = (const char *value) {
			assign(value);
			return *this;
		}

		std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override {
			auto state = std::make_shared<State<std::string>>(node);
			push_back(state);
			return state;
		}

		std::string to_string() const override {
			return value;
		}

	};

	/// @brief Boolean agent.
	template <>
	class UDJAT_API Agent<bool> : public Abstract::Agent {
	private:

		/// @brief Agent value.
		bool value;

		/// @brief Agent states.
		std::vector<std::shared_ptr<State<bool>>> states;

	protected:

		std::shared_ptr<Abstract::State> stateFromValue() const override {
			for(auto state : states) {
				if(state->compare(this->value))
					return state;
			}
			return super::stateFromValue();
		}

		/// @brief Insert state.
		void push_back(std::shared_ptr<State<bool>> state) {
			states.push_back(state);
		}

	public:
		Agent(const pugi::xml_node &node) : Abstract::Agent(node), value(node.attribute("value").as_bool()) {
		}

		Agent(const char *name = "") : Abstract::Agent(name), value(false) {
		}

		Agent(const char *name, bool v) : Abstract::Agent(name), value(v) {
		}

		bool set(const bool value) {

			if(value == this->value)
				return updated(false);

			this->value = value;
			return updated(true);
		}

		inline Agent & operator = (const bool value) {
			set(value);
			return *this;
		}

		bool get() const noexcept {
			return value;
		}

		/// @brief Insert State.
		std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) {
			auto state =std::make_shared<State<bool>>(node);
			push_back(state);
			return state;
		}

		std::string to_string() const override {
			return (value ? "yes" : "no");
		}

	};


 }
