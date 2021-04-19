/**
 * @file src/include/udjat/state.h
 *
 * @brief Declare the agent states.
 *
 * @author perry.werneck@gmail.com
 *
 */

#ifndef UDJAT_STATE_H_INCLUDED

	#define UDJAT_STATE_H_INCLUDED

	#include <string>
	#include <pugixml.hpp>
	#include <memory>
	#include <vector>
	#include <mutex>
	#include <functional>
	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <udjat/tools/xml.h>
	#include <udjat/request.h>
	#include <json/value.h>
	#include <cstring>

	namespace Udjat {

		void parse_range(const pugi::xml_node &node, int &from, int &to);
		void parse_range(const pugi::xml_node &node, unsigned int &from, unsigned int &to);

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

				/// @brief Notify on state activation?
				bool notify = false;

			private:

				Level level;		///< @brief State level.
				Quark summary;		///< @brief Message summary.
				Quark body;			///< @brief Message body.
				Quark uri;			///< @brief Web link to this state (Usually used for http exporters).
				time_t activation;	///< @brief Timestamp of the last state activation.

				static Level getLevelFromName(const char *name);

				/// @brief State alerts.
				std::vector<std::shared_ptr<Alert>> alerts;

			public:
				State(const Level l, const char *m, const char *b = "");

				State(const pugi::xml_node &node);

				State(const std::exception &e) : State(critical, e.what()) {
				}

				virtual ~State();

				static const char * to_string(const Level level);

				inline const Quark & getSummary() const {
					return summary;
				}

				inline const Quark & getBody() const {
					return body;
				}

				inline const Quark & getUri() const {
					return uri;
				}

				inline time_t getActivationTime() const noexcept {
					return activation;
				}

				inline void push_back(std::shared_ptr<Alert> alert) {
					alerts.push_back(alert);
				}

				inline Level getLevel() const {
					return this->level;
				}

				void getLevel(Json::Value &value) const;

				inline bool isCritical() const noexcept {
					return this->level >= critical;
				}

				virtual void get(Json::Value &value) const;
				virtual void get(const Request &request, Response &response) const;

				void activate(const Agent &agent) noexcept;
				void deactivate(const Agent &agent) noexcept;


			};

		}


		template <typename T>
		class UDJAT_API State : public Abstract::State {
		private:

			/// @brief State value;
			T from;
			T to;

		public:
			State(const pugi::xml_node &node) : Abstract::State(node) {
				parse_range(node,from,to);
			}

			bool compare(T value) {
				return value >= from && value <= to;
			}

			void get(Json::Value &value) const override {
				Abstract::State::get(value);
				value["value"] = this->from;
			}

		};


		template <>
		class UDJAT_API State<std::string> : public Abstract::State {
		private:

			/// @brief State value;
			std::string value;

		public:
			State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::Attribute(node,"value",false).as_string()) {
			}

			bool compare(const std::string &value) {
				return strcasecmp(this->value.c_str(),value.c_str()) == 0;
			}

			void get(Json::Value &value) const override {
				Abstract::State::get(value);
				value["value"] = this->value;
			}

		};

		template <>
		class UDJAT_API State<bool> : public Abstract::State {
		private:

			/// @brief State value;
			bool value;

		public:
			State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::Attribute(node,"value",false).as_bool()) {
			}

			bool compare(const bool value) {
				return this->value == value;
			}

			void get(Json::Value &value) const override {
				Abstract::State::get(value);
				value["value"] = this->value;
			}

		};

	}

	namespace std {

		inline string to_string(const std::shared_ptr<Udjat::Abstract::State> state) {
			return state->getSummary().c_str();
		}

		inline string to_string(const Udjat::Abstract::State::Level level) {
			return Udjat::Abstract::State::to_string(level);
		}

		inline ostream& operator<< (ostream& os, const std::shared_ptr<Udjat::Abstract::State> state) {
			return os << state->getSummary();
		}

		inline ostream& operator<< (ostream& os, const Udjat::Abstract::State::Level level) {
			return os << Udjat::Abstract::State::to_string(level);
		}

	}

#endif // UDJAT_STATE_H_INCLUDED
