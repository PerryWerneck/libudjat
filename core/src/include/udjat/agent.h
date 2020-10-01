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
	#include <udjat/tools/atom.h>
	#include <udjat/request.h>
	#include <json/value.h>
	#include <cstring>

	namespace Udjat {

		namespace Abstract {

			class UDJAT_API State {
			public:
				enum Level : uint8_t {
						undefined,
						unimportant,
						ready,
						warning,
						error,
						critical		///< @brief Critical level (allways the last one)
				};

				/// @brief Strings com os nomes dos níveis de estado.
				static const char * levelNames[];

			private:

				Level level;
				Atom summary;
				Atom href;			///< @brief Web link to this state (Usually used for http exporter).

				static Level getLevelFromName(const char *name);

			public:
				State(const Level l, const char *m);

				State(const pugi::xml_node &node);

				State(const std::exception &e) : State(critical, e.what()) {
				}

				inline const char * getSummary() const {
					return summary.c_str();
				}

				inline Level getLevel() const {
					return this->level;
				}

				virtual void get(Json::Value &value) const;
				virtual Abstract::Request & get(Abstract::Request &request);

				Json::Value as_json() const;

				void activate(const Agent &agent);
				void deactivate(const Agent &agent);


			};

			class UDJAT_API Agent {
			private:

				friend class Factory::Controller;

				static std::recursive_mutex guard;

				Atom name;

				Agent *parent = nullptr;

				struct {
					time_t last = 0;		///< @brief Timestamp of the last update.
					time_t next = 0;		///< @brief Timestamp of the next update.
					time_t running = 0;		///< @brief Non zero if the update is running.
					time_t timer = 0;		///< @brief Update time (0=No update).
					bool on_demand = false;	///< @brief True if agent should update on request.
				} update;

				std::vector<std::shared_ptr<Agent>> children;

				/// @brief Current state.
				std::shared_ptr<State> state;

			protected:

				/// @brief Activate a new state.
				void activate(std::shared_ptr<State> state) noexcept;

				/// @brief Web link for this agent (Usually used for http exporter).
				Atom href;

				/// @brief Value was updated, compute new state.
				void revalidate();

				/// @brief Run update if required.
				void chk4refresh();

				/// @brief Insert State.
				virtual void append_state(const pugi::xml_node &node);

				/// @brief Find state from agent value.
				virtual std::shared_ptr<Abstract::State> find_state() const;

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

				std::shared_ptr<Agent> find(const char *name);
				std::shared_ptr<Agent> find(const std::vector<std::string> &path);

				void foreach(std::function<void(Agent &agent)> method);
				void foreach(std::function<void(std::shared_ptr<Agent> agent)> method);

				virtual Json::Value as_json();
				virtual Abstract::Request & get(Abstract::Request &request);

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
		void set_root_agent(std::shared_ptr<Abstract::Agent> agent);

		/// @brief Get Agent from path
		std::shared_ptr<Abstract::Agent> find_agent(const char *path);

		/// @brief Wrapper for XML attribute
		class UDJAT_API Attribute : public pugi::xml_attribute {
		public:
			Attribute(const pugi::xml_node &node, const char *name) : pugi::xml_attribute(node.attribute(name)) {
			}

			operator uint32_t() const {
				return as_uint();
			}

			operator int32_t() const {
				return as_int();
			}

			operator bool() const {
				return as_bool();
			}

		};

		template <typename T>
		class UDJAT_API Agent : public Abstract::Agent {
		private:

			/// @brief Agent state
			class State : public Abstract::State {
			private:

				/// @brief State value;
				T value;

			public:
				State(const pugi::xml_node &node) : Abstract::State(node), value((T) Attribute(node,"value")) {
				}

				bool compare(T value) {
					return this->value == value;
				}

			};

			/// @brief Agent value.
			T value;

			/// @brief Agent states.
			std::vector<std::shared_ptr<State>> states;

		protected:

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State>(node));
			}

			std::shared_ptr<Abstract::State> find_state() const override {
				for(auto state : states) {
					if(state->compare(this->value))
						return state;
				}
				return Abstract::Agent::find_state();
			}

			/// @brief Select state by value.

			/// @brief Add value to JSON.
			void get(Json::Value &value) override {
				Abstract::Agent::get(value);
				value["value"] = this->value;
			}

			/// @brief Add value to request.
			Abstract::Request & get(Abstract::Request &request) override {
				return Abstract::Agent::get(request).push(this->getName(),this->value);
			}

		public:
			Agent(Abstract::Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent,node), value((T) Attribute(node,"value")) {
			}

			bool set(const T &value) {

				if(value == this->value)
					return false;

				revalidate();
				return true;
			}

			T get() const noexcept {
				return value;
			}

		};

		template <>
		class Agent<std::string> : public Abstract::Agent {
		private:

			/// @brief Agent state
			class State : public Abstract::State {
			private:

				/// @brief State value;
				std::string value;

			public:
				State(const pugi::xml_node &node) : Abstract::State(node), value(Attribute(node,"value").as_string()) {
				}

				bool compare(const std::string &value) {
					return strcasecmp(this->value.c_str(),value.c_str()) == 0;
				}

			};

			/// @brief Agent value.
			std::string value;

			/// @brief Agent states.
			std::vector<std::shared_ptr<State>> states;

		protected:

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State>(node));
			}

			std::shared_ptr<Abstract::State> find_state() const override {
				for(auto state : states) {
					if(state->compare(this->value))
						return state;
				}
				return Abstract::Agent::find_state();
			}

			/// @brief Select state by value.

			/// @brief Add value to JSON.
			void get(Json::Value &value) override {
				Abstract::Agent::get(value);
				value["value"] = this->value;
			}

			Abstract::Request & get(Abstract::Request &request) override {
				return Abstract::Agent::get(request).push(this->getName(),this->value);
			}

		public:
			Agent(Abstract::Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent,node), value(Attribute(node,"value").as_string()) {
			}

			bool set(const std::string &value) {

				if(value == this->value)
					return false;

				revalidate();
				return true;
			}

			std::string get() const noexcept {
				return value;
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

		inline string to_string(const std::shared_ptr<Udjat::Abstract::State> state) {
			return state->getSummary();
		}

		inline ostream& operator<< (ostream& os, const std::shared_ptr<Udjat::Abstract::State> state) {
			return os << state->getSummary();
		}

	}

#endif // UDJAT_AGENT_H_INCLUDED
