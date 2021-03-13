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

				class Controller;

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

				/// @brief Load child from xml node.
				void load(const pugi::xml_node &node);

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

				/// @brief Insert child agent.
				void insert(std::shared_ptr<Agent> child);

			public:

				/// @brief Agent factory
				class UDJAT_API Factory {
				private:
					Quark name;
					class Controller;

				public:
					Factory(const Quark &name);
					virtual ~Factory();

					const char * c_str() const {
						return name.c_str();
					}

					static bool parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node);

					virtual void parse(Abstract::Agent &parent, const pugi::xml_node &node) const = 0;

				};

				/// @brief Load agents from xml.file
				void load(const pugi::xml_document &doc);

				Agent(const char *name = nullptr, const char *label = nullptr, const char *summary = nullptr);
				Agent(const pugi::xml_node &node);
				virtual ~Agent();

				/// @brief Get root agent.
				static std::shared_ptr<Abstract::Agent> get_root();

				/// @brief Set root agent.
				static std::shared_ptr<Abstract::Agent> set_root(std::shared_ptr<Abstract::Agent> agent);

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

				void foreach(std::function<void(Agent &agent)> method);
				void foreach(std::function<void(std::shared_ptr<Agent> agent)> method);

				virtual void get(Json::Value &value, const bool children = false, const bool state = true);
				virtual void get(const char *name, Json::Value &value);
				virtual void get(const Request &request, Response &response);

				/// @brief Get current state
				inline std::shared_ptr<State> getState() const {
					return this->state;
				}

			};


		}

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

			void get(const char *name, Json::Value &value) override {
				value[name] = Json::Value(this->value);
			}

		public:
			Agent(const pugi::xml_node &node) : Abstract::Agent(node), value((T) Attribute(node,"value")) {
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
				auto value = setup(request,response);
				value["value"] = this->value;
				this->getState()->get(value["state"]);
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

			void get(const char *name, Json::Value &value) override {
				value[name] = Json::Value(this->value);
			}

		public:
			Agent(const pugi::xml_node &node) : Abstract::Agent(node), value(Attribute(node,"value").as_string()) {
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
				auto value = setup(request,response);
				value["value"] = this->value;
				this->getState()->get(value["state"]);
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

		public:
			Agent(const pugi::xml_node &node) : Abstract::Agent(node), value(Attribute(node,"value").as_bool(false)) {
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

			void get(const char *name, Json::Value &value) override {
				value[name] = Json::Value(this->value);
			}

		};

	}

#endif // UDJAT_AGENT_H_INCLUDED
