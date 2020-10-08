#ifndef UDJAT_NOTIFICATION_H_INCLUDED

	#define UDJAT_NOTIFICATION_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/agent.h>
	#include <udjat/state.h>
	#include <functional>

	namespace Udjat {

		class UDJAT_API Notification {
		private:
			Abstract::State::Level 	level;
			Atom					summary;		///< @brief Notification summary.
			Atom					message;		///< @brief Notification message.
			Atom					href;			///< @brief Web link to this notification (if available).

		public:
			Notification(const Abstract::State &state);
			Notification(const Abstract::Agent &agent);
			~Notification();

			inline const char * getSummary() const {
				return summary.c_str();
			}

			inline const char * getMessage() const {
				return message.c_str();
			}

			inline const char * getHref() const {
				return href.c_str();
			}

			void emit() const noexcept;

		};

		/// @brief Insert notification listener.
		insert(std::function<void(const Notification &)> method);

	}


#endif // UDJAT_NOTIFICATION_H_INCLUDED
