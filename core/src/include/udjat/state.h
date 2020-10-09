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
	#include <udjat/tools/atom.h>
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

				/// @brief Strings com os nomes dos níveis de estado.
				static const char * levelNames[];

				/// @brief Notify on state activation?
				bool notify = false;

			private:

				Level level;		///< @brief State level.
				Atom summary;		///< @brief State summary.
				Atom detailed;		///< @brief Detailed message.
				Atom href;			///< @brief Web link to this state (Usually used for http exporters).

				static Level getLevelFromName(const char *name);

				/// @brief State events.
				std::vector<Abstract::Event *> events;

			public:
				State(const Level l, const char *m, const char *d = "");

				State(const pugi::xml_node &node);

				State(const std::exception &e) : State(critical, e.what()) {
				}

				virtual ~State();

				static const char * to_string(const Level level);

				inline const Atom & getSummary() const {
					return summary;
				}

				inline const Atom & getMessage() const {
					return summary;
				}

				inline const Atom & getHRef() const {
					return summary;
				}

				/// @brief Insert and take control of an event.
				/// The event pointer will be deleted with the state.
				inline void push_back(Abstract::Event *event) {
					events.push_back(event);
				}

				inline Level getLevel() const {
					return this->level;
				}

				inline bool isCritical() const noexcept {
					return this->level >= critical;
				}

				virtual void get(Json::Value &value) const;
				virtual Request & get(Request &request);

				Json::Value as_json() const;

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

		};


		template <>
		class UDJAT_API State<std::string> : public Abstract::State {
		private:

			/// @brief State value;
			std::string value;

		public:
			State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::getAttribute(node,"value").as_string()) {
			}

			bool compare(const std::string &value) {
				return strcasecmp(this->value.c_str(),value.c_str()) == 0;
			}
		};

		template <>
		class UDJAT_API State<bool> : public Abstract::State {
		private:

			/// @brief State value;
			bool value;

		public:
			State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::getAttribute(node,"value").as_bool()) {
			}

			bool compare(const bool value) {
				return this->value == value;
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
