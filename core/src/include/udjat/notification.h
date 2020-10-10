#ifndef UDJAT_NOTIFICATION_H_INCLUDED

	#define UDJAT_NOTIFICATION_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/agent.h>
	#include <udjat/state.h>
	#include <functional>

	namespace Udjat {

		class UDJAT_API Notification {
		private:
			class Controller;
			friend class Controller;

		protected:
			Abstract::State::Level 	level;
			Atom					label;			///< @brief Notification label (Agent label).
			Atom					summary;		///< @brief Notification summary.
			Atom					message;		///< @brief Notification message.
			Atom					href;			///< @brief Web link to this notification (if available).

		public:

			inline const char * getSummary() const {
				return summary.c_str();
			}

			inline const char * getMessage() const {
				return message.c_str();
			}

			inline const char * getHref() const {
				return href.c_str();
			}

			/// @brief true if there's someone listening for notifications.
			static bool hasListeners() noexcept;

			void emit() const noexcept;

			/// @brief Insert notification listener.
			static void insert(const std::function<void(const Notification &)> method);

		};


	}


#endif // UDJAT_NOTIFICATION_H_INCLUDED
