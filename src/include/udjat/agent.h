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
 #include <udjat/tools/object.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/agent/level.h>
 #include <udjat/agent/state.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/converters.h>
 #include <udjat/tools/schema.h>
 #include <memory>

 namespace Udjat {

	namespace Abstract {

		class UDJAT_API Agent : public Udjat::Object {
		public:

			/// @brief Agent factory
			class UDJAT_API Factory {
			private:
				const char *name;

			public:
				Factory(const char *name);
				virtual ~Factory();

				inline bool operator==(const char *n) const noexcept {
					return strcasecmp(n,name) == 0;
				}

				/// @brief Probe if this factory can be used with the properties.
				/// @param props The properties.
				/// @return true if the factory recognizes the properties.
				bool probe(const Properties &props) const noexcept;

				/// @brief Create an agent from properties.
				/// @param props Properties for the new agent.
				virtual std::shared_ptr<Abstract::Agent> AgentFactory(const Properties &props) const = 0;

			};

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
				LEVEL_CHANGED		= 0x0100,		///< @brief Agent level has changed.

				ALL 				= 0x011F		///< @brief All events.
			};

			static Event EventFactory(const Udjat::Properties &props, const char *attrname = "event");
			static Event EventFactory(const char *name);

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

			/// @brief Notify state change.
			/// @param state New agent state.
			/// @param activate true if the new state will be activated.
			/// @param message Message for logfile.
			/// @return true if the state was changed.
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

			/// @brief Set failed state from exception.
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
			void on_demand(bool opt = true) noexcept;

			/// @brief Set update timer interval.
			/// @param value New timer interval (0 disable it).
			inline time_t timer(time_t value) noexcept {
				return (update.timer = value);
			}

			/// @brief Convenience method to build custom state.
			/// @param name	State name (should be constant).
			/// @param level The state level.
			/// @param summary State summary, will be expanded with agent properties.
			/// @param summary State body, will be expanded with agent properties.
			std::shared_ptr<Abstract::State> StateFactory(const char *name, const Udjat::Level level, const char *summary = "", const char *body = "");

			/// @brief Clear agent (remove all children)
			void clear();

		public:
			class Controller;
			friend class Controller;

			Agent(const Agent&) = delete;
			Agent& operator=(const Agent &) = delete;
			Agent(Agent &&) = delete;
			Agent & operator=(Agent &&) = delete;

			Agent(const char *name = "", const char *label = "", const char *summary = "");
			Agent(const Properties &props);

			~Agent() override;

			/// @brief Append child object from properties.
			/// @details This method is called by parse_children() for every child.
			/// @param props The children properties.
			/// @return true if the properties were parsed and should be ignored by the caller.
			bool append_child(const Udjat::Properties &props) override;

			/// @brief Insert object.
			bool push_back(std::shared_ptr<Abstract::Object> object) override;

			/// @brief Insert object with attributes.
			bool push_back(const Udjat::Properties &props, std::shared_ptr<Abstract::Object> object) override;

			/// @brief Remove object.
			void remove(std::shared_ptr<Abstract::Object> object);

			/// @brief Insert listener.
			void push_back(const Abstract::Agent::Event event, std::shared_ptr<Activatable> activatable);

			/// @brief Remove listener.
			void remove(std::shared_ptr<Activatable> activatable);

			/// @brief Remove listener.
			void remove(const Abstract::Agent::Event event, std::shared_ptr<Activatable> activatable);

			/// @brief Factory for the default root agent.
			static std::shared_ptr<Agent> RootFactory();

			/// @brief Get root agent.
			static std::shared_ptr<Abstract::Agent> root();

			inline std::vector<std::shared_ptr<Agent>> & agents() noexcept {
				return children.agents;
			}

			inline std::list<std::shared_ptr<Abstract::Object>> & objects() noexcept {
				return children.objects;
			}

			/// @brief Deinitialize agent subsystem.
			static void deinit();

			/// @brief Get update timer interval.
			inline time_t timer() const noexcept {
				return update.timer;
			}

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

			/// @brief Get Agent path.
			std::string path() const;

			/// @brief Is the agent empty?
			/// @return false if the agent have children.
			virtual bool empty() const noexcept;

			/// @brief Start agent.
			virtual void start();

			/// @brief Update agent.
			/// @param ondemand true if the update was requested by user.
			/// @return true if the data was updated.
			virtual bool refresh(bool ondemand = false);

			/// @brief Stop agent and children.
			virtual void stop();

			/// @brief Find child by path.
			/// @param path	Child path.
			/// @param required Launch exception when search fails.
			/// @param autoins Insert default child if not found.
			/// @return Agent pointer (empty if not found).
			virtual std::shared_ptr<Agent> find(const char *path, bool required = true, bool autoins = false);

			/// @brief Get agent properties.
			/// @param value Value to receive the properties.
			Value & get_properties(Value &value) const override;

			/// @brief Get child properties by path.
			/// @param path	Child path.
			/// @param value Object for child properties.
			/// @retval true if the child was found.
			/// @retval false if the child was not found.
			bool get_properties(const char *path, Value &value) const;

			/// @brief Retrieves the schema definition for the agent outputs.
			/// @param path The request path for schema.
			/// @param[out] schema Object populated with the output schema details.
			/// @return True if the object defines an output schema; false otherwise (schema remains unmodified).
			bool output_schema(const char *path, Schema &schema) const noexcept override;

			void for_each(std::function<void(Agent &agent)> method);
			void for_each(std::function<void(std::shared_ptr<Agent> agent)> method);

			virtual void for_each(const std::function<void(const Abstract::State &state)> &method) const;

#if __cplusplus >= 201703L
			inline auto begin() noexcept {
				return children.agents.begin();
			}

			inline auto end() noexcept {
				return children.agents.end();
			}

			inline auto begin() const noexcept {
				return children.agents.begin();
			}

			inline auto end() const noexcept {
				return children.agents.end();
			}
#else
			inline std::vector<std::shared_ptr<Agent>>::iterator begin() noexcept {
				return children.agents.begin();
			}

			inline std::vector<std::shared_ptr<Agent>>::iterator end() noexcept {
				return children.agents.end();
			}

			inline std::vector<std::shared_ptr<Agent>>::const_iterator begin() const noexcept {
				return children.agents.begin();
			}

			inline std::vector<std::shared_ptr<Agent>>::const_iterator end() const noexcept {
				return children.agents.end();
			}
#endif
			/// @brief Get agent value.
			virtual Value & get(Value &value) const;

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

			/// @brief Get current state
			inline std::shared_ptr<State> state() const {
				return this->current_state.selected;
			}

			/// @brief Get current level.
			inline Level level() const {
				return this->current_state.selected->level();
			}

			/// @brief Is agent ready?
			inline bool ready() const {
				return this->current_state.selected->ready();
			}

			/// @brief Create and insert State.
			virtual std::shared_ptr<Abstract::State> StateFactory(const Properties &props);

			/// @brief Get agent property.
			/// @param key The property name.
			/// @param value String to update with the property value.
			/// @return true if the property was found.
			bool get_property(const char *key, std::string &value) const override;

			/// @brief get time of the last modification on this agent.
			/// @return Timestamp of last modification.
			/// @retval 0 The last modification time was not available.
			virtual time_t last_modified() const noexcept;

			/// @brief get expiration time for this agent data.
			/// @retval 0 The expiration time is not available.
			virtual time_t expires() const noexcept;

		};
	}

	template <typename T>
	class UDJAT_API Agent : public Abstract::Agent {
	private:

		/// @brief Agent value.
		T value;

	protected:

		typedef Agent<T> super;

		/// @brief Agent states.
		std::vector<std::shared_ptr<State<T>>> states;

		void for_each(const std::function<void(const Abstract::State &state)> &method) const override {
			for(auto state : states) {
				method(*state);
			}
		}

		/// @brief Insert State with predefined value.
		std::shared_ptr<Abstract::State> StateFactory(const Properties &props, T value) {
			auto state = std::make_shared<State<T>>(props, value);
			states.push_back(state);
			return state;
		}

		/// @brief Insert State with predefined range.
		std::shared_ptr<Abstract::State> StateFactory(const Properties &props, T from, T to) {
			auto state = std::make_shared<State<T>>(props, from, to);
			states.push_back(state);
			return state;
		}

		std::shared_ptr<Abstract::State> computeState() override {
			for(auto state : states) {
				if(state->compare(this->value))
					return state;
			}
			return Abstract::Agent::computeState();
		}

		Udjat::Value & get(Udjat::Value &value) const override {
			return value.set(this->value);
		}

		/// @brief Start with value.
		/// @param value The initial value to set.
		inline void start(const T value) {
			this->value = value;
			Abstract::Agent::start();
		}

	public:

		Agent(const Udjat::Properties &props, const T v = 0) : Abstract::Agent{props}, value{props.get("value",v)} {
		}

		Agent(const char *name, const Properties &props, const T v = 0) : Abstract::Agent{name,props}, value{props.get("value",v)} {
		}

		Agent(const char *name = "") : Abstract::Agent{name}, value{0} {
		}

		Agent(const char *name, const T v) : Abstract::Agent{name}, value{v} {
		}

		friend std::ostream& operator<<(std::ostream& out, const Agent &a) {
			return out << a.value;
		}

		bool set(const T &value) {

			if(value == this->value)
				return updated(false);

			this->value = value;
			return updated(true);
		}

		bool assign(const char *value) override {
			return set(from_string<T>(value));
		}

		inline bool operator ==(const T value) const noexcept {
			return this->value == value;
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
		std::shared_ptr<Abstract::State> StateFactory(const Properties &props) override {
			auto state = std::make_shared<State<T>>(props);
			states.push_back(state);
			return state;
		}

		bool output_schema(const char *path, Schema &schema) const noexcept override {
			Abstract::Agent::output_schema(path,schema);
			schema.append(
				Schema::Item{ "value",	Udjat::Value::TypeFactory<T>() }
			);
			return true;
		}

		std::string to_string() const noexcept override {
			return std::to_string(value);
		}

	};

	template <>
	class UDJAT_API Agent<std::string> : public Abstract::Agent {
	private:

		/// @brief Agent value.
		std::string value;

	protected:

		typedef Agent<std::string> super;

		/// @brief Agent states.
		std::vector<std::shared_ptr<State<std::string>>> states;

		std::shared_ptr<Abstract::State> computeState() override {
			for(auto state : states) {
				if(state->compare(this->value))
					return state;
			}
			return Abstract::Agent::computeState();
		}

		Udjat::Value & get(Udjat::Value &value) const override {
			return value.set(this->value);
		}

	public:
		Agent(const Udjat::Properties &props) : Abstract::Agent{props}, value{props["value"].c_str()} {
		}

		Agent(const char *name = "") : Abstract::Agent(name) {
		}

		Agent(const char *name, const char *v) : Abstract::Agent(name), value(v) {
		}

		friend std::ostream& operator<<(std::ostream& out, const Agent &a) {
			return out << a.value.c_str();
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

		std::shared_ptr<Abstract::State> StateFactory(const Properties &props) override {
			auto state = std::make_shared<State<std::string>>(props);
			states.push_back(state);
			return state;
		}

		bool output_schema(const char *path, Schema &schema) const noexcept override {
			Abstract::Agent::output_schema(path,schema);
			schema.append(
				Schema::Item{ "value",	Udjat::Value::String }
			);
			return true;
		}

		std::string to_string() const noexcept override {
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

		typedef Agent<bool> super;

		std::shared_ptr<Abstract::State> computeState() override {
			for(auto state : states) {
				if(state->compare(this->value))
					return state;
			}
			return Abstract::Agent::computeState();
		}

	public:
		Agent(const Udjat::Properties &props) : Abstract::Agent{props}, value(props.get("value",false)) {
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
		std::shared_ptr<Abstract::State> StateFactory(const Properties &props) {
			auto state =std::make_shared<State<bool>>(props);
			states.push_back(state);
			return state;
		}

		bool output_schema(const char *path, Schema &schema) const noexcept override {
			Abstract::Agent::output_schema(path,schema);
			schema.append(
				Schema::Item{ "value",	Udjat::Value::Boolean }
			);

			return true;
		}

		std::string to_string() const noexcept override {
			return (value ? "yes" : "no");
		}

	};


 }
