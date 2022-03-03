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
 * @file src/include/udjat/agent.h
 *
 * @brief Declare the agent classes
 *
 * @author perry.werneck@gmail.com
 *
 */

#ifndef UDJAT_AGENT_H_INCLUDED

	#define UDJAT_AGENT_H_INCLUDED

	#include <string>
	#include <pugixml.hpp>
	#include <memory>
	#include <vector>
	#include <mutex>
	#include <list>
	#include <functional>
	#include <udjat/defs.h>
	#include <udjat/state.h>
	#include <udjat/tools/quark.h>
	#include <udjat/tools/logger.h>
	#include <udjat/request.h>
	#include <udjat/tools/xml.h>
	#include <udjat/alert.h>
	#include <udjat/tools/converters.h>
	#include <udjat/tools/value.h>
	#include <udjat/tools/object.h>
	#include <cstring>

	namespace Udjat {

		UDJAT_API void parse_value(const pugi::xml_node &node, int &value);
		UDJAT_API void parse_value(const pugi::xml_node &node, unsigned int &value);
		UDJAT_API void parse_value(const pugi::xml_node &node, unsigned short &value);
		UDJAT_API void parse_value(const pugi::xml_node &node, float &value);
		UDJAT_API void parse_value(const pugi::xml_node &node, double &value);
		UDJAT_API void parse_value(const pugi::xml_node &node, unsigned long &value);
		UDJAT_API void parse_value(const pugi::xml_node &node, long &value);

		namespace Abstract {

			class UDJAT_API Agent : public Udjat::Object {
			private:
				static std::recursive_mutex guard;

				Agent *parent = nullptr;

				struct {
					time_t last = 0;		///< @brief Timestamp of the last update.
					time_t next = 0;		///< @brief Timestamp of the next update.
					time_t running = 0;		///< @brief Non zero if the update is running.
					time_t timer = 0;		///< @brief Update time (0=No update).
					time_t failed = 300;	///< @brief Delay when the agent fails to update.
					bool on_demand = false;	///< @brief True if agent should update on request.
				} update;

				struct {
					/// @brief Active state.
					std::shared_ptr<State> active;

					/// @brief State activation.
					time_t activation;

				} current_state;

				/// @brief Agent children.
				std::vector<std::shared_ptr<Agent>> children;

				/// @brief Associated objects.
				std::list<std::shared_ptr<Abstract::Object>> objects;

				/// @brief Child state has changed; compute my new state.
				void onChildStateChange() noexcept;

				/// @brief Enable disable 'running updates' flag.
				void updating(bool running);

			protected:

				/// @brief Allow use of super:: for accessing abstract::agent methods.
				typedef Abstract::Agent super;

				/// @brief Update complete (success or failure).
				/// @param changed true if the value has changed.
				/// @return Value of 'changed'.
				virtual bool updated(bool changed) noexcept;

				/// @brief Level changed.
				virtual void onLevelChange();

				/// @brief Activate a new state.
				/// @return true if the level has changed.
				virtual bool activate(std::shared_ptr<State> state);

				/// @brief Activate an alert.
				void activate(std::shared_ptr<Abstract::Alert> alert) const;

				/// @brief Set failed state from known exception
				void failed(const char *summary, const std::exception &e) noexcept;

				/// @brief Set failed state from errno.
				void failed(const char *summary, int code) noexcept;

				/// @brief Set unexpected failed state.
				void failed(const char *summary, const char *body = "") noexcept;

				/// @brief Run update if required.
				/// @param forward	If true forward update to children.
				/// @return true if the state was refreshed.
				bool chk4refresh(bool forward = false);

				/// @brief Compute state from agent value.
				/// @return Computed state or the default one if agents has no state table.
				virtual std::shared_ptr<Abstract::State> stateFromValue() const;

				/// @brief Set 'on-demand' option.
				void setOndemand() noexcept;

				/// @brief Set agent details on value.
				Value & getProperties(Value &response) const noexcept override;

			public:
				class Controller;

				/// @brief Insert child node.
				void insert(std::shared_ptr<Agent> child);

				/// @brief Insert Alert.
				virtual void push_back(std::shared_ptr<Abstract::Alert> alert);

				/// @brief Insert object.
				void push_back(std::shared_ptr<Abstract::Object> object);

				/// @brief Remove object.
				void remove(std::shared_ptr<Abstract::Object> object);

				Agent(const char *name = "", const char *label = "", const char *summary = "");
				Agent(const pugi::xml_node &node);

				virtual ~Agent();

				/// @brief Get root agent.
				static std::shared_ptr<Abstract::Agent> root();

				UDJAT_DEPRECATED(static std::shared_ptr<Abstract::Agent> get_root());

				/// @brief Load children from xml node.
				/// @brief node XML node with agent attributes.
				void load(const pugi::xml_node &node);

				/// @brief Deinitialize agent subsystem.
				static void deinit();

				/// @brief Get update timer interval.
				inline time_t getUpdateInterval() const noexcept {
					return update.timer;
				}

				/// @brief true if the agent has states.
				// virtual bool hasStates() const noexcept;

				/// @brief Get Agent path.
				std::string path() const;

				/// @brief The agent has children?
				UDJAT_DEPRECATED(bool hasChildren() const noexcept) {
					return ! this->children.empty();
				}

				/// @brief The agent has children?
				/// @return false if the agent have children.
				bool empty() const noexcept {
					return children.empty();
				}

				/// @brief Start agent.
				virtual void start();

				/// @brief Update agent.
				/// @param ondemand true if the update was requested by user query.
				/// @return true if the data was updated.
				virtual bool refresh(bool ondemand);

				/// @brief Update agent.
				/// @return true if the data was updated.
				virtual bool refresh();

				/// @brief Stop agent.
				virtual void stop();

				/// @brief Find child by path.
				/// @param path	Child path.
				/// @param required Launch exception when search fails.
				/// @param autoins Insert default child if not found.
				/// @return Agent pointer.
				virtual std::shared_ptr<Agent> find(const char *path, bool required = true, bool autoins = false);

				void for_each(std::function<void(Agent &agent)> method);
				void for_each(std::function<void(std::shared_ptr<Agent> agent)> method);

				inline void foreach(std::function<void(Agent &agent)> method) {
					for_each(method);
				}

				void foreach(std::function<void(std::shared_ptr<Agent> agent)> method) {
					for_each(method);
				}

				inline std::vector<std::shared_ptr<Agent>>::iterator begin() noexcept {
					return children.begin();
				}

				inline std::vector<std::shared_ptr<Agent>>::iterator end() noexcept {
					return children.end();
				}

				/// @brief Adds cache and update information to the response.
				void head(Response &response);

				/// @brief Get agent value.
				virtual Value & get(Value &value) const;

				virtual void get(Response &response);
				virtual void get(const Request &request, Response &response);
				virtual void get(const Request &request, Report &report);

				/// @brief Get formatted value.
				virtual std::string to_string() const override;

				/// @brief Assign value from string.
				virtual bool assign(const char *value);

				UDJAT_DEPRECATED(inline std::shared_ptr<State> getState() const) {
					return this->current_state.active;
				}

				/// @brief Get current state
				inline std::shared_ptr<State> state() const {
					return this->current_state.active;
				}

				/// @brief Get current level.
				inline Level level() const {
					return this->current_state.active->level();
				}

				/// @brief Create and insert State.
				virtual std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node);

				/// @brief Insert Alert.
				virtual std::shared_ptr<Abstract::Alert> AlertFactory(const pugi::xml_node &node);

				/// @brief Get property from the agent os related objects.
				/// @param key The property name.
				/// @param value String to update with the property value.
				/// @return true if the property was found.
				bool getProperty(const char *key, std::string &value) const noexcept override;

			};

		}

		/// @brief Load XML application definitions.
		/// @param pathname Path to a single xml file or a folder with xml files.
		/// @return Seconds for the next reload.
		UDJAT_API time_t load(const char *pathname);

		/// @brief Load XML application definitions.
		/// @param agent New root agent.
		/// @param pathname Path to a single xml file or a folder with xml files.
		/// @param time_t Seconds for file refresh.
		UDJAT_API time_t load(std::shared_ptr<Abstract::Agent> agent, const char *pathname);

		template <typename T>
		class UDJAT_API Agent : public Abstract::Agent {
		private:

			/// @brief Agent value.
			T value;

			/// @brief Agent states.
			std::vector<std::shared_ptr<State<T>>> states;

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
			void push_back(std::shared_ptr<State<T>> state) {
				states.push_back(state);
			}

		public:
			Agent(const pugi::xml_node &node) : Abstract::Agent(node) {
				parse_value(node,value);
			}

			Agent(const pugi::xml_node &node, const T v) : Abstract::Agent(node), value(v) {
			}

			Agent(const char *name = "") : Abstract::Agent(name), value(0) {
			}

			Agent(const char *name, T v) : Abstract::Agent(name), value(v) {
			}

			bool set(const T &value) {

				if(value == this->value)
					return updated(false);

				this->value = value;
#ifdef DEBUG
				info() << Logger::Message("Value set to {}",this->value) << std::endl;
#endif // DEBUG
				return updated(true);
			}

			T get() const noexcept {
				return value;
			}

			bool assign(const char *value) override {
				T new_value;
				to_value(value,new_value);
				return set(new_value);
			}

			//bool hasStates() const noexcept override {
			//	return !states.empty();
			//}

			/// @brief Insert State.
			std::shared_ptr<Abstract::State> StateFactory(const pugi::xml_node &node) override {
				auto state =std::make_shared<State<T>>(node);
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

			//bool hasStates() const noexcept override {
			//	return !states.empty();
			//}

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


#endif // UDJAT_AGENT_H_INCLUDED
