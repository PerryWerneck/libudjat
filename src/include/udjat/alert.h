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
 #include <udjat/tools/url.h>
 #include <udjat/tools/object.h>
 #include <memory>

 namespace Udjat {

	namespace Abstract {

		/// @brief Abstract alert.
		class UDJAT_API Alert : public NamedObject {
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

				struct {
					bool restarting = false;
					time_t running = 0;
				} state;

				/// @brief Schedule next alert.
				void next() noexcept;

			protected:

				/// @brief Alert name
				std::string name;

				/// @brief Alert was send.
				void success() noexcept;

				/// @brief Alert was not send.
				void failed() noexcept;

			public:
				Activation();
				virtual ~Activation();

				virtual const char * c_str() const noexcept;

				inline std::shared_ptr<Alert> alert() const {
					return alertptr;
				};

				/// @brief Is the activation running?
				inline bool running() const noexcept {
					return state.running != 0;
				}

				/// @brief Emit alert.
				virtual void emit() const;

				/// @brief Get activation info.
				virtual Value & getProperties(Value &value) const noexcept;

			};

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
			constexpr Alert(const char *name) : NamedObject(name) {
			}

			/// @brief Create alert for xml description.
			/// @param node XML node with the alert description.
			/// @param defaults Section on configuration file for the alert default options (can be overrided by xml attribute 'settings-from'.
			Alert(const pugi::xml_node &node, const char *defaults = "alert-defaults");
			virtual ~Alert();

			/// @brief Activate an alert.
			static void activate(std::shared_ptr<Alert> alert, const std::function<void(std::string &str)> &expander);

			/// @brief Activate and alert without expansion.
			static void activate(std::shared_ptr<Alert> alert);

			/// @brief Deactivate an alert.
			void deactivate();

			/// @brief Get alert info.
			Value & getProperties(Value &value) const noexcept override;

		};

	}

	/// @brief Default alert (based on URL and payload).
	class UDJAT_API Alert : public Abstract::Alert {
	protected:

		const char *url = "";
		HTTP::Method action = HTTP::Get;
		const char *payload = "";

		/// @brief URL based alert activation.
		class UDJAT_API Activation : public Abstract::Alert::Activation {
		protected:
			std::string url;
			HTTP::Method action;
			std::string payload;

		public:
			Activation(const std::string &u, const HTTP::Method a, const std::string &p);
			Activation(const Alert &alert, const std::function<void(std::string &str)> &expander);
			void emit() const override;
			const char * c_str() const noexcept override;

		};

		std::shared_ptr<Abstract::Alert::Activation> ActivationFactory(const std::function<void(std::string &str)> &expander) const;

	public:

		class UDJAT_API Factory : public Udjat::Factory {
		public:
			Factory();
			bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const override;
			bool parse(Abstract::State &parent, const pugi::xml_node &node) const override;
		};

		constexpr Alert(const char *name, const char *u, const HTTP::Method a = HTTP::Get, const char *p = "") : Abstract::Alert(name), url(u), action(a), payload(p) {
		}

		Alert(const pugi::xml_node &node, const char *defaults = "alert-defaults");

		/// @brief Activate a single alert with the default settings.
		static void activate(const char *name, const char *url, const char *action = "get", const char *payload = "");

		/// @brief Get alert info.
		Value & getProperties(Value &value) const noexcept override;

	};

 }

 namespace std {

	inline string to_string(const Udjat::Abstract::Alert &alert) {
			return alert.c_str();
	}

	inline ostream& operator<< (ostream& os, Udjat::Abstract::Alert &alert) {
			return os << alert.c_str();
	}

 }
