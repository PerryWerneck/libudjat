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
 #include <udjat/tools/parse.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/value.h>
 #include <udjat/agent/level.h>
 #include <udjat/agent/state.h>
 #include <mutex>
 #include <list>

 namespace Udjat {

	namespace Abstract {

		class UDJAT_API Agent : public Udjat::Object {
		public:

			enum Event : uint8_t {
				VALUE_CHANGED,		///< @brief Agent value has changed.
				STATE_CHANGED,		///< @brief Agent state has changed.

			};

			class UDJAT_API EventListener {
			private:
				friend class Agent;

				const void *id;
				Event event;

			public:
				constexpr EventListener(const Event e, const void *i = 0) : id(i),event(e) {
				}

				virtual void trigger() noexcept = 0;

			};

		private:
			static std::recursive_mutex guard;

			Agent *parent = nullptr;	///< @brief Agent parent.

			struct {
				time_t last = 0;		///< @brief Timestamp of the last update.
				time_t next = 0;		///< @brief Timestamp of the next update.
				time_t running = 0;		///< @brief Non zero if the update is running.
				time_t timer = 0;		///< @brief Update time (0=No update).
				time_t failed = 300;	///< @brief Delay when the agent fails to update.
				bool on_demand = false;	///< @brief True if agent should update on request.
#ifndef _WIN32
				short sigdelay = -1;	///< @brief Delay (in seconds) after the update signal (-1 no signal).
#endif // !WIN32
			} update;

			struct {
				/// @brief Active state.
				std::shared_ptr<State> active;

				/// @brief State activation.
				time_t activation;

			} current_state;

			/// @brief Agent children.
			struct {

				/// @brief Agent children.
				std::vector<std::shared_ptr<Agent>> agents;

				/// @brief Object children.
				std::list<std::shared_ptr<Abstract::Object>> objects;

			} children;

			/// @brief Event listeners.
			std::list<std::shared_ptr<EventListener>> listeners;

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

			/// @brief Send event to listeners.
			void notify(const Event event);

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

			/// @brief Reset time for the next update (force a refresh in the next cicle if seconds=0).
			void requestRefresh(time_t seconds = 0);

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
			void insert(std::shared_ptr<Abstract::Agent> child);

			/// @brief Insert child node.
			void push_back(std::shared_ptr<Abstract::Agent> child);

			/// @brief Insert object.
			void push_back(std::shared_ptr<Abstract::Object> object);

			/// @brief Insert Alert.
			virtual void push_back(std::shared_ptr<Abstract::Alert> alert);

			/// @brief Create and insert child.
			/// @param type The agent type.
			/// @param node XML agent definitions.
			/// @return true if the child was created.
			bool push_back(const char *type, const pugi::xml_node &node);

			/// @brief Create and insert child from XML definition.
			/// @param node XML agent definitions.
			/// @return true if the child was created.
			bool push_back(const pugi::xml_node &node);

			/// @brief Remove object.
			void remove(std::shared_ptr<Abstract::Object> object);

			Agent(const char *name = "", const char *label = "", const char *summary = "");
			Agent(const pugi::xml_node &node);

			virtual ~Agent();

			/// @brief Get root agent.
			static std::shared_ptr<Abstract::Agent> root();

			inline std::vector<std::shared_ptr<Agent>> & agents() noexcept {
				return children.agents;
			}

			inline std::list<std::shared_ptr<Abstract::Object>> & objects() noexcept {
				return children.objects;
			}

			/// @brief Load children from xml node.
			/// @brief node XML node with agent attributes.
			void load(const pugi::xml_node &node);

			/// @brief Deinitialize agent subsystem.
			static void deinit();

			/// @brief Get update timer interval.
			inline time_t getUpdateInterval() const noexcept {
				return update.timer;
			}

			/// @brief Get update timer interval.
			inline time_t updatetimer() const noexcept {
				return update.timer;
			}

			/// @brief Get Agent path.
			std::string path() const;

			/// @brief The agent has children?
			UDJAT_DEPRECATED(bool hasChildren() const noexcept) {
				return !children.agents.empty();
			}

			/// @brief The agent has children?
			/// @return false if the agent have children.
			bool empty() const noexcept {
				return children.agents.empty();
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

			/*
			inline void foreach(std::function<void(Agent &agent)> method) {
				for_each(method);
			}

			void foreach(std::function<void(std::shared_ptr<Agent> agent)> method) {
				for_each(method);
			}
			*/

			inline std::vector<std::shared_ptr<Agent>>::iterator begin() noexcept {
				return children.agents.begin();
			}

			inline std::vector<std::shared_ptr<Agent>>::iterator end() noexcept {
				return children.agents.end();
			}

			/// @brief Adds cache and update information to the response.
			void head(ResponseInfo &response);

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

 }

 /*
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


		}

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


	}


#endif // UDJAT_AGENT_H_INCLUDED

*/
