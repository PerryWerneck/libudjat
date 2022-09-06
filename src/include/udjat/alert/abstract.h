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
 #include <udjat/tools/string.h>
 #include <udjat/agent/level.h>
 #include <memory>

 namespace Udjat {

	namespace Alert {

		class Controller;
		class Activation;

	}

	namespace Abstract {

		/// @brief Abstract alert.
		class UDJAT_API Alert : public NamedObject {
		private:

			/// @brief Alert options.
			struct {
				bool verbose = false;
			} options;

			/// @brief Alert limits.
			struct {
				unsigned int min = 1;			///< @brief How many success emissions after deactivation or sleep?
				unsigned int max = 3;			///< @brief How many retries (success+fails) after deactivation or sleep?
			} retry;

			/// @brief Alert timers.
			struct {
				unsigned int start = 0;			///< @brief Seconds to wait before first activation.
				unsigned int interval = 60;		///< @brief Seconds to wait on every try.
				unsigned int busy = 60;			///< @brief Seconds to wait if the alert is busy when activated.
			} timers;

			/// @brief Restart timers.
			struct {
				unsigned int failed = 14400;	///< @brief Seconds to wait for reactivate after a failed activation.
				unsigned int success = 0;		///< @brief Seconds to wait for reactivate after a successful activation.
			} restart;

		public:

			constexpr Alert(const char *name) : NamedObject(name) {
			}

			/// @brief Create alert for xml description.
			/// @param node XML node with the alert description.
			/// @param defaults Section on configuration file for the alert default options (can be overrided by xml attribute 'settings-from'.
			Alert(const pugi::xml_node &node, const char *defaults = "alert-defaults");
			virtual ~Alert();

			/// @brief Is the alert in verbose mode?
			inline bool verbose() const noexcept {
				return options.verbose;
			}

			/// @brief Deactivate an alert.
			void deactivate();

			/// @brief Get alert info.
			Value & getProperties(Value &value) const noexcept override;

			/// @brief Alert activation.
			class UDJAT_API Activation {
			private:
				friend class Udjat::Alert::Controller;

				const void * id = nullptr;

				struct {
					unsigned int min;
					unsigned int max;
				} retry;

				struct {
					bool verbose = true;
					Udjat::Level level = Udjat::unimportant;
				} options;

				struct {
					unsigned int interval = 0;
					unsigned int busy = 0;
					unsigned int failed = 14400;
					unsigned int success = 0;
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

				/// @brief Activation name
				std::string name;

				/// @brief Activation description.
				std::string description;

				/// @brief Just emit alert, no update on emission data.
				virtual void emit();

			public:
				Activation(const Alert *alert);
				virtual ~Activation();

				/// @brief Emit alert, update timers block until completed.
				/// @return true if the alert emission was ok.
				bool run() noexcept;

				/// @brief Rename activation.
				/// @param new_name New activation name.
				inline void rename(const char *new_name) noexcept {
					this->name = new_name;
				}

				/// @brief Set description.
				inline void set(const char *descr) noexcept {
					description = descr;
				}

				/// @brief Set level.
				inline void set(const Udjat::Level level) noexcept {
					options.level = level;
				}

				/// @brief Set object (expand ${} on strings).
				virtual void set(const Abstract::Object &object);

				inline bool verbose() const noexcept {
					return options.verbose;
				}

				/// @brief Is the activation running?
				inline bool running() const noexcept {
					return state.running != 0;
				}

				/// @brief Get activation info.
				virtual Value & getProperties(Value &value) const noexcept;

				std::ostream & info() const;
				std::ostream & warning() const;
				std::ostream & error() const;

			};

			/// @brief Create and activation for this alert.
			virtual std::shared_ptr<Activation> ActivationFactory() const;

		};

	}

	/// @brief Start alert activation.
	UDJAT_API void start(std::shared_ptr<Abstract::Alert::Activation> activation);

	/// @brief Create an alert from XML description;
	/// @param parent Parent object, usually an agent, scanned for alert attributes.
	/// @param node XML description of the alert.
	/// @param type Alert type ('url' or 'script' for internal ones, factory name for module based alerts).
	/// @return Pointer to the new alert.
	UDJAT_API std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const pugi::xml_node &node, const char *type = "default");

 }
