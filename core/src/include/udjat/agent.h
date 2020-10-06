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
	#include <udjat/tools/atom.h>
	#include <udjat/request.h>
	#include <udjat/tools/xml.h>
	#include <json/value.h>
	#include <cstring>

	namespace Udjat {

		namespace Abstract {

			class UDJAT_API Agent {
			private:

				friend class Factory::Controller;

				static std::recursive_mutex guard;

				Atom name;

				Agent *parent = nullptr;

				struct {
					time_t last = 0;		///< @brief Timestamp of the last update.
					time_t expires = 0;		///< @brief Is the current state valid?
					time_t next = 0;		///< @brief Timestamp of the next update.
					time_t running = 0;		///< @brief Non zero if the update is running.
					time_t timer = 0;		///< @brief Update time (0=No update).
					bool on_demand = false;	///< @brief True if agent should update on request.
				} update;

				std::vector<std::shared_ptr<Agent>> children;

				/// @brief Current state.
				std::shared_ptr<State> state;

				/// @brief Child state has changed; compute my new state.
				void onChildStateChange() noexcept;

			protected:

				/// @brief Activate a new state.
				void activate(std::shared_ptr<State> state) noexcept;

				/// @brief Set failed state from known exception
				void failed(const std::exception &e, const char *message) noexcept;

				/// @brief Set unexpected failed state.
				void failed(const char *message) noexcept;

				/// @brief Web link for this agent (Usually used for http exporter).
				Atom href;

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
				Request & setup(Request &request);

			public:
				Agent(Agent *parent = nullptr);
				Agent(Agent *parent, const pugi::xml_node &node);
				virtual ~Agent();

				/// @brief Get Agent name
				const char * getName() const noexcept {
					return this->name.c_str();
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

				virtual Request & get(const char *name, Request &request);
				virtual Request & get(Request &request);

				/// @brief Get current state
				inline std::shared_ptr<State> getState() {
					chk4refresh();
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

			bool set(const T &value) {

				if(value == this->value)
					return false;

				onValueChange();
				return true;
			}

			T get() const noexcept {
				return value;
			}

			/// @brief Add value to request.
			Request & get(const char *name, Request &request) override {
				return setup(request).push(name,this->value);
			}

			Request & get(Request &request) override {
				return setup(request).push("value",this->value);
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

			Request & get(Request &request) override {
				return setup(request).push("value",this->value);
			}

			Request & get(const char *name, Request &request) override {
				return setup(request).push(name,this->value);
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

			Request & get(Request &request) override {
				return setup(request).push("value",this->value);
			}

			Request & get(const char *name, Request &request) override {
				return setup(request).push(name,this->value);
			}

		};

	}

	namespace std {

		inline string to_string(const Udjat::Abstract::Agent &agent) {
			return agent.getName();
		}

		inline ostream& operator<< (ostream& os, const Udjat::Abstract::Agent &agent) {
			return os << agent.getName();
		}

		inline string to_string(const std::shared_ptr<Udjat::Abstract::Agent> agent) {
			return agent->getName();
		}

		inline ostream& operator<< (ostream& os, const std::shared_ptr<Udjat::Abstract::Agent> agent) {
			return os << agent->getName();
		}

	}

#endif // UDJAT_AGENT_H_INCLUDED
