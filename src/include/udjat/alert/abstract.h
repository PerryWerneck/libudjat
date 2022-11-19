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
 #include <udjat/alert.h>
 #include <udjat/agent/level.h>
 #include <memory>

 namespace Udjat {

	namespace Abstract {

		/// @brief Abstract alert.
		class UDJAT_API Alert : public NamedObject {
		private:

			friend class Udjat::Alert::Activation;

			/// @brief Alert options.
			struct {
				bool verbose = false;
				bool asyncronous = true;
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

		protected:

			/// @brief Get alert payload.
			/// @param XML node to scan for payload.
			/// @return Payload as Quark;
			const char * getPayload(const pugi::xml_node &node);

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

			/// @brief Is the alert in asyncronous mode?
			inline bool asyncronous() const noexcept {
				return options.asyncronous;
			}

			/// @brief Deactivate an alert.
			void deactivate();

			/// @brief Get alert info.
			Value & getProperties(Value &value) const noexcept override;

			/// @brief Create and activation for this alert.
			virtual std::shared_ptr<Udjat::Alert::Activation> ActivationFactory() const;

		};

	}

 }
