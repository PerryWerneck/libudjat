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
	#include <functional>
	#include <udjat/defs.h>
	#include <udjat/state.h>
	#include <udjat/tools/quark.h>
	#include <udjat/tools/logger.h>
	#include <udjat/request.h>
	#include <udjat/tools/xml.h>
	#include <json/value.h>
	#include <cstring>

	namespace Udjat {

		namespace Abstract {

			class UDJAT_API Agent : public Logger {
			private:

				friend class Factory::Controller;

				static std::recursive_mutex guard;

				Agent *parent = nullptr;

				struct {
					time_t last = 0;		///< @brief Timestamp of the last update.
					time_t expires = 0;		///< @brief Is the current state valid?
					time_t next = 0;		///< @brief Timestamp of the next update.
					time_t running = 0;		///< @brief Non zero if the update is running.
					time_t timer = 0;		///< @brief Update time (0=No update).
					bool on_demand = false;	///< @brief True if agent should update on request.
					bool notify = false;	///< @brief Notify when updated?
				} update;

				std::vector<std::shared_ptr<Agent>> children;

				/// @brief Current state.
				std::shared_ptr<State> state;

				/// @brief Agent events.
				std::vector<Abstract::Event *> events;

				/// @brief Child state has changed; compute my new state.
				void onChildStateChange() noexcept;

			protected:

				/// @brief Activate a new state.
				/// @return true if the level has changed.
				bool activate(std::shared_ptr<State> state) noexcept;

				/// @brief Set failed state from known exception
				void failed(const std::exception &e, const char *message) noexcept;

				/// @brief Set unexpected failed state.
				void failed(const char *message) noexcept;

				/// @brief Agent label.
				Quark label;

				/// @brief Agent summary.
				Quark summary;

				/// @brief Web link for this agent (HTTP API).
				Quark uri;

				/// @brief URL for an agent icon (HTTP API)
				Quark icon;

				/// @brief Value has changed, compute my new state.
				void onValueChange() noexcept;

				/// @brief Run update if required.
				/// @param forward	If true forward update to children.
				/// @return true if the state was refreshed.
				bool chk4refresh(bool forward = false);

				/// @brief Insert State.
				virtual void append_state(const pugi::xml_node &node);

				/// @brief Find state from agent value.
				virtual std::shared_ptr<Abstract::State> find_state() const;

				/// @brief Setup (adds cache and update information)
				/// @return Node for value.
				Json::Value & setup(const Request &request, Response &response);

			protected:

			public:
				Agent(Agent *parent = nullptr);
				Agent(Agent *parent, const pugi::xml_node &node);
				virtual ~Agent();

				/// @brief true if the agent has states.
				virtual bool hasOwnStates() const noexcept;

				/// @brief Insert and take control of an event.
				/// The event pointer will be deleted with the agent.
				inline void push_back(Abstract::Event *event) {
					events.push_back(event);
				}

				inline const Quark & getUri() const noexcept {
					return uri;
				}

				inline const Quark & getIcon() const noexcept {
					return icon;
				}

				inline const Quark & getLabel() const noexcept {
					return label;
				}

				/// @brief Start agent.
				virtual void start();

				/// @brief Update agent.
				virtual void refresh();

				/// @brief Stop agent.
				virtual void stop();

				std::shared_ptr<Agent> find(const char *path);
				//std::shared_ptr<Agent> find(const std::vector<std::string> &path);

				void foreach(std::function<void(Agent &agent)> method);
				void foreach(std::function<void(std::shared_ptr<Agent> agent)> method);

				virtual Json::Value as_json();

				virtual void get(const Request &request, Response &response);

				/// @brief Get current state
				inline std::shared_ptr<State> getState() {
					chk4refresh();
					return this->state;
				}

				inline std::shared_ptr<State> getState() const {
					return this->state;
				}

				/// @brief Get agent value.
				virtual void get(Json::Value &value);

			};


		}


		/// @brief Set root agent
		void UDJAT_API set_root_agent(std::shared_ptr<Abstract::Agent> agent);

		/// @brief Get root agent
		std::shared_ptr<Abstract::Agent> UDJAT_API get_root_agent();

		/// @brief Get Agent from path
		std::shared_ptr<Abstract::Agent> UDJAT_API find_agent(const char *path);

		template <typename T>
		class UDJAT_API Agent : public Abstract::Agent {
		private:

			/// @brief Agent value.
			T value;

			/// @brief Agent states.
			std::vector<std::shared_ptr<State<T>>> states;

		protected:

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State<T>>(node));
			}

			std::shared_ptr<Abstract::State> find_state() const override {
				for(auto state : states) {
					if(state->compare(this->value))
						return state;
				}
				return Abstract::Agent::find_state();
			}

			/// @brief Add value to JSON.
			void get(Json::Value &value) override {
				Abstract::Agent::get(value);
				value["value"] = this->value;
			}

		public:
			Agent(Abstract::Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent,node), value((T) Attribute(node,"value")) {
			}

			Agent(T v) : Abstract::Agent(), value(v) {
			}

			bool set(const T &value) {

				if(value == this->value)
					return false;

				this->value = value;
#ifdef DEBUG
				info("Value set to {}",this->value);
#endif // DEBUG
				onValueChange();
				return true;
			}

			T get() const noexcept {
				return value;
			}

			/// @brief Add value to request.
			void get(const Request &request, Response &response) override {
				this->setup(request,response)["value"] = this->value;
			}

			bool hasOwnStates() const noexcept override {
				return !states.empty();
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

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State<std::string>>(node));
			}

			std::shared_ptr<Abstract::State> find_state() const override {
				for(auto state : states) {
					if(state->compare(this->value))
						return state;
				}
				return Abstract::Agent::find_state();
			}

			/// @brief Add value to JSON.
			void get(Json::Value &value) override {
				Abstract::Agent::get(value);
				value["value"] = this->value;
			}

		public:
			Agent(Abstract::Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent,node), value(Attribute(node,"value").as_string()) {
			}

			bool set(const std::string &value) {

				if(value == this->value)
					return false;

				this->value = value;
				onValueChange();
				return true;
			}

			std::string get() const noexcept {
				return value;
			}

			void get(const Request &request, Response &response) override {
				this->setup(request,response)["value"] = this->value;
			}

			bool hasOwnStates() const noexcept override {
				return !states.empty();
			}

		};

		///
		template <>
		class UDJAT_API Agent<bool> : public Abstract::Agent {
		private:

			/// @brief Agent value.
			bool value;

			/// @brief Agent states.
			std::vector<std::shared_ptr<State<bool>>> states;

		protected:

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State<bool>>(node));
			}

			std::shared_ptr<Abstract::State> find_state() const override {
				for(auto state : states) {
					if(state->compare(this->value))
						return state;
				}
				return Abstract::Agent::find_state();
			}

			/// @brief Add value to JSON.
			void get(Json::Value &value) override {
				Abstract::Agent::get(value);
				value["value"] = this->value;
			}

		public:
			Agent(Abstract::Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent,node), value(Attribute(node,"value").as_bool(false)) {
			}

			bool set(const bool value) {

				if(value == this->value)
					return false;

				this->value = value;
				onValueChange();
				return true;
			}

			bool get() const noexcept {
				return value;
			}

		};

	}

#endif // UDJAT_AGENT_H_INCLUDED
