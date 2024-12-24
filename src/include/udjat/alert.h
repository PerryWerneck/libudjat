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
 #include <memory>
 #include <udjat/tools/object.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/activatable.h>

 namespace Udjat {

	class UDJAT_API Alert : public Activatable {
	private:
		class Controller;
		friend class Controller;
	protected:

		typedef Udjat::Alert super;

		/// @brief Clear activation parameters.
		/// @param active True if the alert is being activated.
		virtual void reset(bool active) noexcept;

		/// @brief Alert limits.
		struct {
			unsigned int min = 1;		///< @brief How many success emissions after deactivation or sleep?
			unsigned int max = 3;		///< @brief How many retries (success+fails) after deactivation or sleep?
		} retry;

		/// @brief Alert timers.
		struct {
			unsigned int start = 0;		///< @brief Seconds to wait before first activation.
			unsigned int interval = 60;	///< @brief Seconds to wait on every try.
		} timers;

		/// @brief Restart timers.
		struct {
			unsigned int failed = 0;	///< @brief Seconds to wait for reactivate after a failed activation.
			unsigned int success = 0;	///< @brief Seconds to wait for reactivate after a successful activation.
		} restart;

		/// @brief Alert activation.
		struct {
			bool running = false;		///< @brief True if the alert is running.
			unsigned int suceeded = 0;	///< @brief Number of successful activations.
			unsigned int failed = 0;	///< @brief Number of failed activations.
			time_t next = 0;			///< @brief Next activation time (0 = inactive).	
		} activation;

		/// @brief Emit an alert.
		/// @return 0 on success, error code when failed.
		virtual int emit() = 0;

		void failed(const char *message) noexcept;
		void success() noexcept;

	public:
		/// @brief Alert Factory.
		class UDJAT_API Factory {
		private:
			const char *factory_name;

		public:
			Factory(const char *name);
			virtual ~Factory();

			inline bool operator==(const char *n) const noexcept {
				return strcasecmp(n,factory_name) == 0;
			}

			inline const char *name() const noexcept {
				return factory_name;
			}
			
			/// @brief Create an agent from XML node.
			/// @param node XML definition for the new alert.
			virtual std::shared_ptr<Alert> AlertFactory(const Abstract::Object &parent, const XML::Node &node) const;

			static std::shared_ptr<Alert> build(const Abstract::Object &parent, const XML::Node &node);

		};

		Alert(const char *name);
		Alert(const XML::Node &node);

		virtual ~Alert();

		/// @brief Activate an alert.
		/// @return true if the alert was activated, false if already active.
		bool activate() noexcept override;

		/// @brief Deactivate an alert.
		/// @return true if the alert was deactivated, false if already inactive.
		bool deactivate() noexcept override;

		/// @brief Activate an alert, scheduling the next activation.
		void activate(time_t next) noexcept;


	};


 }
