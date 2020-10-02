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

				Level level;		///< @brief State level.
				Atom summary;		///< @brief State summary.
				Atom detailed;		///< @brief Detailed message.
				Atom href;			///< @brief Web link to this state (Usually used for http exporter).

				static Level getLevelFromName(const char *name);

			public:
				State(const Level l, const char *m, const char *d = "");

				State(const pugi::xml_node &node);

				State(const std::exception &e) : State(critical, e.what()) {
				}

				inline const char * getSummary() const {
					return summary.c_str();
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

				void activate(const Agent &agent);
				void deactivate(const Agent &agent);


			};

		}

	}

	namespace std {

		inline string to_string(const std::shared_ptr<Udjat::Abstract::State> state) {
			return state->getSummary();
		}

		inline ostream& operator<< (ostream& os, const std::shared_ptr<Udjat::Abstract::State> state) {
			return os << state->getSummary();
		}

	}

#endif // UDJAT_STATE_H_INCLUDED
