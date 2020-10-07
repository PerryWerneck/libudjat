#ifndef UDJAT_EVENT_H_INCLUDED

	#define UDJAT_EVENT_H_INCLUDED

	#include <udjat/defs.h>
	#include <pugixml.hpp>
	#include <udjat/tools/atom.h>
	#include <memory>

	namespace Udjat {

		namespace Abstract {

			/// @brief Abstract class for events.
			class UDJAT_API Event {
			private:
				class Controller;
				friend class Controller;

				Atom name;

				struct {
					size_t count	= 0;		///< @brief How many retries in the current cycle?
					time_t last		= 0;		///< @brief Timestamp of last retry.
					time_t next		= 0;		///< @brief Timestamp of next retry.
					time_t first	= 0;		///< @brief Time for first retry.
					time_t interval	= 0;		///< @brief Time for next retries.
					size_t limit	= 0;		///< @brief Maximum number of retries.
				} retry;

			public:
				Event(const Atom &name);
				Event(const char *name);
				Event(const pugi::xml_node &node);

				inline const char * c_str() const noexcept {
					return name.c_str();
				}

				virtual ~Event();

				/// @brief Signal emitted when agent value changes.
				/// @return true if the event was processed, false to retry.
				virtual bool emit(const Abstract::Agent &agent, bool level_has_changed = false);

				/// @brief Signal emitted on state activation/deactivation
				/// @return true if the event was processed, false to retry.
				virtual bool emit(const Abstract::Agent &agent, const Abstract::State &state, bool active);

			};

		}

	}

	namespace std {

		inline ostream& operator<< (ostream& os, const Udjat::Abstract::Event &event) {
			return os << event.c_str();
		}

	}


#endif // UDJAT_EVENT_H_INCLUDED
