#ifndef UDJAT_EVENT_H_INCLUDED

	#define UDJAT_EVENT_H_INCLUDED

	#include <udjat/defs.h>
	#include <pugixml.hpp>
	#include <udjat/tools/atom.h>
	#include <udjat/tools/logger.h>
	#include <memory>
	#include <functional>

	namespace Udjat {

		namespace Abstract {

			/// @brief Abstract class for events.
			class UDJAT_API Event : public Logger {
			private:
				class Controller;
				friend class Controller;

				struct {
					time_t first	= 0;		///< @brief Time for first retry.
					time_t interval	= 60;		///< @brief Time for next retries.
					size_t limit	= 3;		///< @brief Maximum number of retries.
				} retry;

			protected:

				/// @brief Fire event, no agent or state.
				virtual void emit();

				/// @brief Fire event for agent/state.
				virtual void emit(const Abstract::Agent &agent, const Abstract::State &state);

				/// @brief Start customized event.
				void set(const Abstract::Agent *agent, const Abstract::State *state, const std::function<void(const Abstract::Agent &agent, const Abstract::State &state)> callback);

			public:
				Event(const Atom &name);
				Event(const char *name);
				Event(const pugi::xml_node &node);

				/// @brief Call method on every active event.
				static void forEach(const std::function<void(const Abstract::Event &event, const Abstract::Agent &agent, const Abstract::State &state, time_t last, time_t next, size_t count)> call);

				inline const char * c_str() const noexcept {
					return name.c_str();
				}

				virtual ~Event();

				/// @brief Stop event
				void clear();

				/// @brief Notify agent value change.
				virtual void set(const Abstract::Agent &agent, bool level_has_changed = false);

				/// @brief Notify activation/deactivation of state
				/// @return true if the event was processed, false to retry.
				virtual void set(const Abstract::Agent &agent, const Abstract::State &state, bool active);

			};

		}

	}


#endif // UDJAT_EVENT_H_INCLUDED
