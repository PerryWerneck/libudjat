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

				/// @brief State name
				const char * name = "";

				static const char *levelnames[];

			private:

				/// @brief State level.
				Level level = unimportant;

				/// @brief Message summary.
				const char * summary = "";

				/// @brief Message body.
				const char * body = "";

				/// @brief Web link to this state (Usually used for http exporters).
				const char * uri = "";

				/// @brief Timestamp of the last state activation.
				time_t activation = 0;

				/// @brief Translate level from string.
				static Level getLevelFromName(const char *name);

				/// @brief State alerts.
				// std::vector<std::shared_ptr<Alert>> alerts;

				/// @brief Parse XML node
				void set(const pugi::xml_node &node);

			protected:

				virtual void activate(const Agent &agent,std::vector<std::shared_ptr<Alert>> &alerts) noexcept;
				virtual void deactivate(const Agent &agent,std::vector<std::shared_ptr<Alert>> &alerts) noexcept;

			public:

				/// @brief Create state using the strings without conversion.
				constexpr State(const char *n, const Level l = Level::unimportant, const char *s = "", const char *b = "")
					: name(n), level(l), summary(s), body(b) { }

				// constexpr State(const char *name, const Level level = Level::unimportant, const char *summary = "", const char *body = "");

				/// @brief Create state (convert strings to Quarks).
				State(const Level l, const Quark &summary, const Quark &body = "");

				/// @brief Create state (convert strings to Quarks).
				State(const Level l, const char *summary, const char *body = "");

				/// @brief Create state from xml node)
				State(const pugi::xml_node &node);

				State(const std::exception &e) : State(critical, e.what()) {
				}

				virtual ~State();

				operator Quark() const {
					return name;
				}

				static const char * to_string(const Level level);

				inline const char * getSummary() const {
					return summary;
				}

				inline const char * getBody() const {
					return body;
				}

				inline const char * getUri() const {
					return uri;
				}

				inline time_t getActivationTime() const noexcept {
					return activation;
				}

				virtual void push_back(std::shared_ptr<Alert> alert);

				//inline void push_back(std::shared_ptr<Alert> alert) {
				//	alerts.push_back(alert);
				//}

				inline Level getLevel() const {
					return this->level;
				}

				void getLevel(Json::Value &value) const;

				inline bool isCritical() const noexcept {
					return this->level >= critical;
				}

				virtual void get(Json::Value &value) const;
				virtual void get(const Request &request, Response &response) const;

				virtual void activate(const Agent &agent) noexcept;
				virtual void deactivate(const Agent &agent) noexcept;

				/// @brief Expand ${} tags on string.
				virtual void expand(std::string &text) const;

			};

		}


		template <typename T>
		class UDJAT_API State : public Abstract::State {
		private:

			/// @brief State value;
			T from;
			T to;

			std::vector<std::shared_ptr<Alert>> alerts;

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

			void push_back(std::shared_ptr<Alert> alert) {
				alerts.push_back(alert);
			}

			void activate(const Abstract::Agent &agent) noexcept override {
				Abstract::State::activate(agent,alerts);
			}

			void deactivate(const Abstract::Agent &agent) noexcept override {
				Abstract::State::activate(agent,alerts);
			}

		};


		template <>
		class UDJAT_API State<std::string> : public Abstract::State {
		private:

			/// @brief State value;
			std::string value;

			std::vector<std::shared_ptr<Alert>> alerts;

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

			void push_back(std::shared_ptr<Alert> alert) {
				alerts.push_back(alert);
			}

			void activate(const Abstract::Agent &agent) noexcept override {
				Abstract::State::activate(agent,alerts);
			}

			void deactivate(const Abstract::Agent &agent) noexcept override {
				Abstract::State::deactivate(agent,alerts);
			}

		};

		template <>
		class UDJAT_API State<bool> : public Abstract::State {
		private:

			/// @brief State value;
			bool value;

			std::vector<std::shared_ptr<Alert>> alerts;

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

			void push_back(std::shared_ptr<Alert> alert) {
				alerts.push_back(alert);
			}

			void activate(const Abstract::Agent &agent) noexcept override {
				Abstract::State::activate(agent,alerts);
			}

			void deactivate(const Abstract::Agent &agent) noexcept override {
				Abstract::State::activate(agent,alerts);
			}
		};

	}

	namespace std {

		inline string to_string(const std::shared_ptr<Udjat::Abstract::State> state) {
			return state->getSummary();
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
