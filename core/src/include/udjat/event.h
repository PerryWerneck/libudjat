#ifndef UDJAT_EVENT_H_INCLUDED

	#define UDJAT_EVENT_H_INCLUDED

	#include <udjat/defs.h>
	#include <pugixml.hpp>
	#include <memory>

	namespace Udjat {

		namespace Abstract {

			/// @brief Abstract class for events.
			class UDJAT_API Event {
			private:
				Atom name;

			public:
				Event(const Atom &name);
				Event(const pugi::xml_node &node);

				inline const char * c_str() const noexcept {
					return name.c_str();
				}

				virtual ~Event();

				/// @brief Signal emitted when agent value changes.
				virtual void emit(const Abstract::Agent &agent, bool level_has_changed);

				/// @brief Signal emitted on state activation/deactivation
				virtual void emit(const Abstract::Agent &agent, const Abstract::State &state, bool active);

			};

		}

	}

	namespace std {

		inline ostream& operator<< (ostream& os, const Udjat::Abstract::Event &event) {
			return os << event.c_str();
		}

	}


#endif // UDJAT_EVENT_H_INCLUDED
