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
	#include <udjat/tools/object.h>
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

			critical		///< @brief Critical level (always the last one)

		};

		/// @brief Get level from string.
		UDJAT_API Level LevelFactory(const char *name);

		/// @brief Get level from XML node.
		UDJAT_API Level LevelFactory(const pugi::xml_node &node);

		namespace Abstract {

			class UDJAT_API State : public Udjat::Object {
			public:

				/// @brief Notify on state activation?
				bool notify = false;

			private:

				/// @brief Parse XML node
				void set(const pugi::xml_node &node);

				/// @brief State alerts.
				std::vector<std::shared_ptr<Abstract::Alert>> alerts;

			protected:

				struct Properties {

					/// @brief State level.
					Level level = unimportant;

					/// @brief Message body.
					const char * body = "";

				} properties;

			public:

				/// @brief Create state using the strings without conversion.
				State(const char *name, const Level level = Level::unimportant, const char *summary = "", const char *body = "");

				/// @brief Create state from xml node
				State(const pugi::xml_node &node);

				std::string to_string() const override {
					return Object::properties.summary;
				}

				virtual ~State();

				inline const char * body() const noexcept {
					return properties.body;
				}

				/// @brief Get the state level.
				/// @return The state level.
				/// @see Level
				inline Level level() const noexcept {
					return properties.level;
				}

				/// @brief Is this state a critical one?
				/// @return true if the state is critical.
				inline bool critical() const noexcept {
					return level() >= Level::critical;
				}

				/// @brief Check if this state is a problem.
				/// @return true if the state is not a problem.
				inline bool ready() const noexcept {
					return level() <= Level::ready;
				}

				virtual void get(const Request &request, Response &response) const;

				virtual void activate(const Agent &agent) noexcept;
				virtual void deactivate(const Agent &agent) noexcept;

				/// @brief Get property.
				/// @param key The property name.
				/// @param value String to update with the property value.
				/// @return true if the property is valid.
				bool getProperty(const char *key, std::string &value) const noexcept override;

				/// @brief Get the state properties.
				/// @brief Value to receive the properties.
				/// @return The save value from arguments.
				Value & getProperties(Value &value) const noexcept override;

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

		UDJAT_API std::shared_ptr<Abstract::State> StateFactory(const std::exception &e, const char *summary);

	}

	namespace std {

		const char * to_string(const Udjat::Level level);

		inline ostream& operator<< (ostream& os, const Udjat::Level level) {
			return os << to_string(level);
		}

	}

#endif // UDJAT_STATE_H_INCLUDED
