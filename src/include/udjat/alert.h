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
 #include <udjat/factory.h>
 #include <memory>

 namespace Udjat {

	namespace Abstract {

		/// @brief Abstract alert.
		class UDJAT_API Alert {
		private:
			class Controller;
			friend class Controller;

		protected:

			/// @brief Alert activation.
			class UDJAT_API Activation {
			private:
				friend class Controller;

				std::shared_ptr<Alert> alertptr;

				struct {
					time_t last = 0;
					time_t next = 0;
				} timers;

				struct {
					unsigned int success = 0;
					unsigned int failed = 0;
				} count;

				void checkForSleep(const char *msg) noexcept;

				bool restarting = false;
				time_t running = 0;

				/// @brief Schedule next alert.
				void next() noexcept;

			protected:

				/// @brief Alert was send.
				void success() noexcept;

				/// @brief Alert was not send.
				void failed() noexcept;

			public:
				Activation();
				virtual ~Activation();

				inline const char * name() const noexcept {
					return alertptr->c_str();
				}

				inline std::shared_ptr<Alert> alert() const {
					return alertptr;
				};

				/// @brief Emit alert.
				virtual void emit() const;
			};

			/// @brief Alert name.
			const char *name = "alert";

			/// @brief Alert limits.
			struct {
				size_t min = 1;				///< @brief How many success emissions after deactivation or sleep?
				size_t max = 3;				///< @brief How many retries (success+fails) after deactivation or sleep?
			} retry;

			/// @brief Alert timers.
			struct {
				time_t start = 0;			///< @brief Seconds to wait before first activation.
				time_t interval = 60;		///< @brief Seconds to wait on every try.
				time_t busy = 60;			///< @brief Seconds to wait if the alert is busy when activated.
			} timers;

			/// @brief Restart timers.
			struct {
				time_t failed = 14400;		///< @brief Seconds to wait for reactivate after a failed activation.
				time_t success = 0;			///< @brief Seconds to wait for reactivate after a successful activation.
			} restart;

			/// @brief Create and activation object for this alert.
			/// @param expander lambda for alert parameters expansion.
			virtual std::shared_ptr<Activation> ActivationFactory(const std::function<void(std::string &str)> &expander) const = 0;

		public:
			constexpr Alert(const char *n) : name(n) {
			}

			/// @brief Create alert for xml description.
			/// @param node XML node with the alert description.
			/// @param defaults Section on configuration file for the alert default options (can be overrided by xml attribute 'settings-from'.
			Alert(const pugi::xml_node &node, const char *defaults = "alert-defaults");
			virtual ~Alert();

			inline const char * c_str() const noexcept {
				return name;
			}

			/// @brief Activate an alert.
			static void activate(std::shared_ptr<Alert> alert, const std::function<void(std::string &str)> &expander);

			/// @brief Activate and alert without expansion.
			static void activate(std::shared_ptr<Alert> alert);

			/// @brief Deactivate an alert.
			void deactivate();

		};

	}

	/// @brief Default alert (based on URL and payload).
	class UDJAT_API Alert : public Abstract::Alert {
	protected:
		const char *url = "";
		const char *action = "";
		const char *payload = "";

		static const char * expand(const char *value, const pugi::xml_node &node, const char *section);

		std::shared_ptr<Abstract::Alert::Activation> ActivationFactory(const std::function<void(std::string &str)> &expander) const;

	public:

		class Factory : public Udjat::Factory {
		public:
			Factory();
			bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const override;
			bool parse(Abstract::State &parent, const pugi::xml_node &node) const override;
		};

		constexpr Alert(const char *name, const char *u, const char *a = "get", const char *p = "") : Abstract::Alert(name), url(u), action(a), payload(p) {
		}

		Alert(const pugi::xml_node &node, const char *defaults = "alert-defaults");

		/// @brief Activate a single alert with the default settings.
		static void activate(const char *name, const char *url, const char *action = "get", const char *payload = "");

	};


 }

