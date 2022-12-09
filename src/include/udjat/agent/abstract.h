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
 #include <udjat/tools/activatable.h>
 #include <mutex>
 #include <list>

 namespace Udjat {

	namespace Abstract {

		class UDJAT_API Agent : public Udjat::Object {
		public:

			enum Event : uint16_t {
				STARTED				= 0x0001,		///< @brief Agent was started.
				STOPPED				= 0x0002,		///< @brief Agent was stopped.
				STATE_CHANGED		= 0x0004,		///< @brief Agent state has changed.
				UPDATE_TIMER		= 0x0008,		///< @brief Agent update starts.
				VALUE_CHANGED		= 0x0010,		///< @brief Agent value has changed.
				VALUE_NOT_CHANGED	= 0x0020,		///< @brief Agent value has not changed.
				UPDATED				= 0x0030,		///< @brief Agent was updated.

				READY				= 0x0040,		///< @brief Agent is ready.
				NOT_READY			= 0x0080,		///< @brief Agent is not ready.
				LEVEL_CHANGED		= 0x00C0,		///< @brief Agent level has changed.

				ALL 				= 0x001F		///< @brief All events.
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
				std::shared_ptr<State> selected;

				enum Activation : uint8_t {
					StateWasSet,			///< @brief The current state was set (usually when a parent remove a forwarded state).
					StateWasActivated,		///< @brief This state was activated.
					StateWasForwarded,		///< @brief This state was forwarded from parent (disable agent updates).
				} activation = StateWasSet;

				time_t timestamp = 0;

				inline void set(std::shared_ptr<State> state) noexcept {
					selected = state;
					timestamp = time(0);
					activation = StateWasSet;
				}

				inline void activate(std::shared_ptr<State> state) noexcept {
					selected = state;
					timestamp = time(0);
					activation = StateWasActivated;
				}

				inline bool activated() const noexcept {
					return (activation == StateWasActivated);
				}

				inline bool forwarded() const noexcept {
					return (activation == StateWasForwarded);
				}

			} current_state;

			/// @brief Set forwarded state on agent and children.
			void forward(std::shared_ptr<State> state) noexcept;

			/// @brief Agent children.
			struct {

				/// @brief Agent children.
				std::vector<std::shared_ptr<Agent>> agents;

				/// @brief Object children.
				std::list<std::shared_ptr<Abstract::Object>> objects;

			} children;

			struct Listener {
				const Abstract::Agent::Event event;
				std::shared_ptr<Activatable> activatable;

				Listener(const Abstract::Agent::Event e, std::shared_ptr<Activatable> a) : event{e}, activatable{a} {
				}

			};

			/// @brief Event listeners.
			std::list<Listener> listeners;

			/// @brief Load agent properties from XML node.
			void setup_properties(const pugi::xml_node &node) noexcept;

			/// @brief Load states from XML node.
			void setup_states(const pugi::xml_node &node) noexcept;

			/// @brief Load alerts from XML node.
			void setup_alerts(const pugi::xml_node &node) noexcept;

			/// @brief Load children from XML node.
			void setup_children(const pugi::xml_node &node) noexcept;

			/// @brief Activate agent state.
			//void activate() noexcept;

			/// @brief Deactivate agent state (if needed).
			//void deactivate() noexcept;

			/// @brief Notify state change.
			/// @param state New agent state.
			/// @param activate if true the new state will be activated.
			/// @param message Message for logfile.
			bool onStateChange(std::shared_ptr<State> state, bool activate, const char *message);

		protected:

			/// @brief Allow use of super:: for accessing abstract::agent methods.
			typedef Abstract::Agent super;

			/// @brief Update complete (success or failure).
			/// @param changed true if the value has changed.
			/// @return Value of 'changed'.
			virtual bool updated(bool changed) noexcept;

			/// @brief Send event to listeners.
			void notify(const Event event);

			/// @brief Set agent state.
			/// @return true if the state has changed.
			virtual bool set(std::shared_ptr<State> state);

			/// @brief Activate a new state.
			/// @return true if the level has changed.
			virtual bool UDJAT_DEPRECATED(activate(std::shared_ptr<State> state));

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
			void chk4refresh(bool forward = false) noexcept;

			/// @brief Compute state from agent value.
			/// @return Computed state or the default one if agents has no state table.
			virtual std::shared_ptr<Abstract::State> computeState();

			/// @brief Set 'on-demand' option.
			void setOndemand() noexcept;

			/// @brief Set agent details on value.
			Value & getProperties(Value &response) const noexcept override;

			/// @brief Set update timer interval.
			/// @param value New timer interval (0 disable it).
			inline time_t timer(time_t value) noexcept {
				return (update.timer = value);
			}

		public:
			class Controller;

			/// @brief Insert child node.
			void UDJAT_DEPRECATED(insert(std::shared_ptr<Abstract::Agent> child));

			/// @brief Insert child node.
			void push_back(std::shared_ptr<Abstract::Agent> child);

			/// @brief Insert object.
			void push_back(std::shared_ptr<Abstract::Object> object);

			/// @brief Insert Alert.
			virtual void push_back(std::shared_ptr<Abstract::Alert> alert);

			/// @brief Insert Alert with XML definition.
			virtual void push_back(const pugi::xml_node &node, std::shared_ptr<Abstract::Alert> alert);

			/// @brief Insert listener.
			void push_back(const Abstract::Agent::Event event, std::shared_ptr<Activatable> activatable);

			/// @brief Remove listener.
			void remove(const Abstract::Agent::Event event, std::shared_ptr<Activatable> activatable);

			/// @brief Remove listener.
			void remove(std::shared_ptr<Activatable> activatable);

			/// @brief Create and insert child.
			/// @param type The agent type.
			/// @param node XML agent definitions.
			/// @return true if the child was created.
			bool ChildFactory(const char *type, const pugi::xml_node &node);

			/// @brief Create and insert child from XML definition.
			/// @param node XML agent definitions.
			/// @return true if the child was created.
			bool ChildFactory(const pugi::xml_node &node);

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

			/// @brief Setup agent from XML node.
			/// @see setup_properties
			/// @see setup_states
			/// @see setup_alerts
			/// @see setup_children
			/// @brief node XML node with agent and children definitions.
			void setup(const pugi::xml_node &node) override;

			/// @brief Deinitialize agent subsystem.
			static void deinit();

			/// @brief Get update timer interval.
			inline time_t timer() const noexcept {
				return update.timer;
			}

			/// @brief Reset time for the next update (force a refresh in the next cicle if seconds=0).
			/// @param seconds Seconds for next refresh.
			void UDJAT_DEPRECATED(requestRefresh(time_t seconds = 0));

			/// @brief Set time for the next update (force a refresh in the next cicle if seconds=0).
			/// @param seconds Seconds for next refresh.
			/// @see reset
			/// @return Update timestamp after change.
			time_t sched_update(time_t seconds = 0);

			/// @brief Reset timestamp for the next update;
			/// @param value Value for next update.
			/// @see sched_update
			/// @return Update timestamp after change.
			time_t reset(time_t timestamp);

			UDJAT_DEPRECATED(inline time_t getUpdateInterval() const noexcept) {
				return update.timer;
			}

			/// @brief Get update timer interval.
			UDJAT_DEPRECATED(inline time_t updatetimer() const noexcept) {
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

			bool operator==(const Abstract::State &state) const noexcept;

			virtual void get(Response &response);
			virtual void get(Report &report);
			virtual void get(const Request &request, Response &response);
			virtual void get(const Request &request, Report &report);

			/// @brief Get formatted value.
			virtual std::string to_string() const noexcept override;

			const char * summary() const noexcept override;
			const char * icon() const noexcept override;
			const char * label() const noexcept override;

			/// @brief Get smart pointer.
			std::shared_ptr<Agent> to_shared_ptr();

			/// @brief Enqueue task.
			size_t push(const std::function<void(std::shared_ptr<Agent> agent)> &method);

			/// @brief Assign value from string.
			virtual bool assign(const char *value);

			UDJAT_DEPRECATED(inline std::shared_ptr<State> getState() const) {
				return this->current_state.selected;
			}

			/// @brief Get current state
			inline std::shared_ptr<State> state() const {
				return this->current_state.selected;
			}

			/// @brief Get current level.
			inline Level level() const {
				return this->current_state.selected->level();
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

