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
	#include <udjat/alert.h>
	#include <udjat/tools/value.h>
	#include <cstring>

	namespace Udjat {

		UDJAT_API void parse_range(const pugi::xml_node &node, int &from, int &to);
		UDJAT_API void parse_range(const pugi::xml_node &node, unsigned int &from, unsigned int &to);
		UDJAT_API void parse_range(const pugi::xml_node &node, unsigned short &from, unsigned short &to);
		UDJAT_API void parse_range(const pugi::xml_node &node, float &from, float &to);
		UDJAT_API void parse_range(const pugi::xml_node &node, double &from, double &to);
		UDJAT_API void parse_range(const pugi::xml_node &node, unsigned long &from, unsigned long &to);
		UDJAT_API void parse_range(const pugi::xml_node &node, long &from, long &to);

		/// @brief Parse	byte range, convert values "TB', 'GB', 'MB', 'KB' to byes.
		/// @param node		XML node to extract values.
		/// @param from		Minor value in bytes.
		/// @param to 		Major value in bytes.
		void parse_byte_range(const pugi::xml_node &node, unsigned long long &from, unsigned long long &to);

		/// @brief Alert/state level.
		// TODO: Convert to class.
		enum Level : uint8_t {
			undefined,
			unimportant,
			ready,
			warning,
			error,

			critical		///< @brief Critical level (allways the last one)

		};

		namespace Abstract {

			class UDJAT_API State {
			public:

				/// @brief Notify on state activation?
				bool notify = false;

				/// @brief State name
				const char * name = "";

				static const char *levelnames[];

			private:

				/// @brief State level.
				Level level = unimportant;

				/// @brief Web link to this state (Usually used for http exporters).
				const char * uri = "";

				/// @brief Translate level from string.
				static Level getLevelFromName(const char *name);

				/// @brief Parse XML node
				void set(const pugi::xml_node &node);

				/// @brief State alerts.
				std::vector<std::shared_ptr<Abstract::Alert>> alerts;

			protected:
				/// @brief Message summary.
				const char * summary = "";

				/// @brief Message body.
				const char * body = "";

			public:

				static std::shared_ptr<Abstract::State> get(const char *summary, const std::exception &e);

				/// @brief Create state using the strings without conversion.
				State(const char *n, const Level l = Level::unimportant, const char *s = "", const char *b = "")
					: name(n), level(l), summary(s), body(b) { }

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

				inline const char * getName() const {
					return name;
				}

				inline const char * getSummary() const {
					return summary;
				}

				inline const char * getBody() const {
					return body;
				}

				inline const char * getUri() const {
					return uri;
				}

				inline Level getLevel() const {
					return this->level;
				}

				Udjat::Value & getLevel(Udjat::Value &value) const;

				inline bool isCritical() const noexcept {
					return this->level >= critical;
				}

				inline bool isReady() const noexcept {
					return this->level <= ready;
				}

				virtual void get(Udjat::Value &value) const;
				virtual void get(const Request &request, Response &response) const;

				virtual void activate(const Agent &agent) noexcept;
				virtual void deactivate(const Agent &agent) noexcept;

				/// @brief Expand ${} tags on string.
				virtual std::string & expand(std::string &text) const;

				/// @brief Insert alert.
				inline void append(std::shared_ptr<Abstract::Alert> alert) {
					alerts.push_back(alert);
				}

			};

		}

		template <typename T>
		class UDJAT_API State : public Abstract::State {
		protected:

			/// @brief Minimum value for state activation.
			T from;

			/// @brief Maximum value for state activation.
			T to;

		public:
			State(const char *name, const T f, const T t, const Level level, const char *summary = "", const char *body = "")
					: Abstract::State(name,level,summary,body), from(f),to(t) { }

			State(const char *name, const T value, const Level level, const char *summary = "", const char *body = "")
					: Abstract::State(name,level,summary,body), from(value),to(value) { }

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
			State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::Attribute(node,"value",false).as_string()) {
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
			State(const pugi::xml_node &node) : Abstract::State(node),value(Udjat::Attribute(node,"value",false).as_bool()) {
			}

			bool compare(const bool value) {
				return this->value == value;
			}

		};

	}

	namespace std {

		inline string to_string(const std::shared_ptr<Udjat::Abstract::State> state) {
			return state->getSummary();
		}

		inline string to_string(const Udjat::Level level) {
			return Udjat::Abstract::State::to_string(level);
		}

		inline ostream& operator<< (ostream& os, const std::shared_ptr<Udjat::Abstract::State> state) {
			return os << state->getSummary();
		}

		inline ostream& operator<< (ostream& os, const Udjat::Level level) {
			return os << Udjat::Abstract::State::to_string(level);
		}

	}

#endif // UDJAT_STATE_H_INCLUDED
