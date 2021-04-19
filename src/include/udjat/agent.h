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

				/// @brief Agent alerts.
				std::vector<std::shared_ptr<Alert>> alerts;

				/// @brief Child state has changed; compute my new state.
				void onChildStateChange() noexcept;

				/// @brief Search for attribute.
				static const pugi::xml_attribute & attribute(const pugi::xml_node &node, const char *name, bool upsearch = true);

			protected:

				/// @brief Load children from xml node.
				void load(const pugi::xml_node &node);

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

				/// @brief Find state from agent value.
				virtual std::shared_ptr<Abstract::State> find_state() const;

				/// @brief Setup (adds cache and update information)
				/// @return Node for value.
				Json::Value & setup(const Request &request, Response &response);

			public:

				/// @brief Load agents from xml.file
				void load(const pugi::xml_document &doc);

				/// @brief Insert child agent.
				void insert(std::shared_ptr<Agent> child);

				Agent(const char *name = nullptr, const char *label = nullptr, const char *summary = nullptr);
				virtual ~Agent();

				/// @brief Get root agent.
				static std::shared_ptr<Abstract::Agent> get_root();

				/// @brief Set root agent.
				static std::shared_ptr<Abstract::Agent> set_root(std::shared_ptr<Abstract::Agent> agent);

				/// @brief true if the agent has states.
				virtual bool hasOwnStates() const noexcept;

				inline void push_back(std::shared_ptr<Alert> alert) {
					alerts.push_back(alert);
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

				/// @brief Find child by path.
				/// @param path	Child path.
				/// @param required Launch exception when search fails.
				/// @param autoins Insert default child if not found.
				/// @return Agent pointer.
				virtual std::shared_ptr<Agent> find(const char *path, bool required = true, bool autoins = false);

				void foreach(std::function<void(Agent &agent)> method);
				void foreach(std::function<void(std::shared_ptr<Agent> agent)> method);

				virtual void get(Json::Value &value, const bool children = false, const bool state = true);
				virtual void get(const char *name, Json::Value &value);
				void get(const Request &request, Response &response);

				/// @brief Get current state
				inline std::shared_ptr<State> getState() const {
					return this->state;
				}

				/// @brief Insert state.
				std::shared_ptr<Abstract::State> push_back(std::shared_ptr<Abstract::State> state);

				/// @brief Insert State.
				virtual void append_state(const pugi::xml_node &node);

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
			Agent(const char *name = nullptr, const char *label = nullptr, const char *summary = nullptr) : Abstract::Agent(name,label,summary), value(0) {
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

			bool hasOwnStates() const noexcept override {
				return !states.empty();
			}

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State<T>>(node));
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
			Agent(const char *name = nullptr, const char *label = nullptr, const char *summary = nullptr) : Abstract::Agent(name,label,summary) {
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

			bool hasOwnStates() const noexcept override {
				return !states.empty();
			}

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State<std::string>>(node));
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

			std::shared_ptr<Abstract::State> find_state() const override {
				for(auto state : states) {
					if(state->compare(this->value))
						return state;
				}
				return Abstract::Agent::find_state();
			}

		public:
			Agent(const char *name = nullptr, const char *label = nullptr, const char *summary = nullptr) : Abstract::Agent(name,label,summary), value(false) {
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

			/// @brief Insert State.
			void append_state(const pugi::xml_node &node) override {
				states.push_back(std::make_shared<State<bool>>(node));
			}

		};

	}

#endif // UDJAT_AGENT_H_INCLUDED
