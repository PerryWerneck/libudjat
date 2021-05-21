/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>
 #include <udjat/tools/quark.h>
 #include <udjat/alert.h>
 #include <udjat/request.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	/// @brief Alert.
	class UDJAT_API Alert : public Logger {
	private:

		/// @brief The Alert controller.
		class Controller;
		friend class Controller;

		size_t events = 0;

		/// @brief Activates on every value change.
		bool activate_on_value_change = true;

		struct {
			size_t min = 1;			///< @brief How many success activations after deactivation or sleep?
			size_t max = 3;			///< @brief How many retries (success+fails) after deactivation or sleep?
			time_t start = 0;		///< @brief Seconds to wait before first activation.
			time_t interval = 60;	///< @brief Seconds to wait on every try.

			/// @brief Restart timers.
			struct {
				time_t failed = 14400;	///< @brief Seconds to wait for reactivate after a failed activation.
				time_t success = 86400;	///< @brief Seconds to wait for reactivate after a successfull activation.
			} restart;

		} retry;

	protected:

		/// @brief Formatted data for sending.
		class UDJAT_API Event : public Logger {
		private:
			friend class Alert;

			/// @brief Mutex for serialization
			static std::mutex guard;

			/// @brief The alert description.
			Alert *parent = nullptr;

			/// @brief Event label.
			Quark label;

			/// @brief Event summary.
			Quark summary;

			/// @brief Event body.
			Quark body;

			/// @brief Web link for this event.
			Quark uri;

			/// @brief URL for this event.
			Quark icon;

			/// @brief Level for this event.
			Abstract::State::Level level;

			/// @brief Is the event running?
			time_t running = 0;

			/// @brief Is the event restarting?
			bool restarting = false;

			/// @brief Control of alerts.
			struct {
				size_t success = 0;		///< @brief How many successful sends in the current cycle.
				size_t failed = 0;		/// @brief How many failed sends in the current cycle.
				time_t last = 0;		///< @brief Last try.
				time_t next = 0;		///< @brief Next try (0 = disabled).
			} alerts;

			/// @brief Restart cicle.
			void reset() {
				alerts.success = alerts.failed = 0;
			}

			/// @brief Update timer to next event.
			void next();

			/// @brief Enqueue event, update counter and timestamp for next.
			static void enqueue(std::shared_ptr<Alert::Event> event);

			/// @brief The maximum number of activation was reached.
			void checkForSleep(const char *msg);

			/// @brief Alert was emitted.
			void success();

			/// @brief Alert was failed.
			void failed();

		public:
			Event();
			Event(const Quark &name);
			Event(const Abstract::Agent &agent, const Abstract::State &state);
			virtual ~Event();

			/// @brief Get event description.
			virtual const char * getDescription() const = 0;

			/// @brief Disable event.
			void disable();

			/// @brief Emit alert.
			virtual void alert(size_t current, size_t total) = 0;

			/// @brief Get information about the event.
			virtual void get(Json::Value &value) const;

		};

		/// @brief Activate alert.
		virtual void activate(const Abstract::Agent &agent, const Abstract::State &state) = 0;

		/// @brief Activate alert event.
		/// Register the supplied event to be 'fired' from alert controller.
		void activate(std::shared_ptr<Event> event);

		/// @brief Deactivate alert; remove all active events from this alert.
		void deactivate() const;

	public:

		/// @brief Get configuration file section for default values.
		static std::string getConfigSection(const pugi::xml_node &node, const char *type = nullptr);

		Alert(const Quark &name);
		Alert(const char *name);
		Alert(const pugi::xml_node &node, const char *type = nullptr);
		virtual ~Alert();

		/// @brief List alerts.
		static void getInfo(Response &response);

		// void insert(Event *event);
		// void remove(Event *event);

		/// @brief Initialize alert subsystem.
		static void init();

		/// @brief Agent value has changed.
		virtual void set(const Abstract::Agent &agent, bool level_has_changed);

		/// @brief State state has changed.
		virtual void set(const Abstract::Agent &agent, const Abstract::State &state, bool active);

	};

 }

