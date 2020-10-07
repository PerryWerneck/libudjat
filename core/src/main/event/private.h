#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/defs.h>
	#include <udjat/event.h>
	#include <iostream>
	#include <mutex>
	#include <list>

	using namespace std;

	namespace Udjat {

		class Abstract::Event::Controller {
		private:
			static recursive_mutex guard;

			Controller();

			struct Action {

				size_t count	= 0;		///< @brief How many retries in the current cycle?
				time_t last		= 0;		///< @brief Timestamp of last retry.
				time_t next		= 0;		///< @brief Timestamp of next retry.

				Abstract::Event *event;
				const Abstract::Agent *agent;
				const Abstract::State *state;
				const std::function<void(const Abstract::Agent &agent, const Abstract::State &state)> call;

				Action(Abstract::Event *e,const Abstract::Agent *a,const Abstract::State *s,const std::function<void(const Abstract::Agent &agent, const Abstract::State &state)> c)
					: event(e), agent(a), state(s), call(c) {
				}

			};

			list<Action> actions;

			void emit(Action &action) noexcept;

		public:
			static Controller & getInstance();
			~Controller();

			void insert(Abstract::Event *event, const Abstract::Agent *agent, const Abstract::State *state, const std::function<void(const Abstract::Agent &agent, const Abstract::State &state)> callback);
			void remove(Abstract::Event *event);

		};

	}

#endif // PRIVATE_H_INCLUDED
