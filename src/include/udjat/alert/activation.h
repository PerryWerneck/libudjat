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
 #include <udjat/alert/abstract.h>
 #include <memory>

 namespace Udjat {

	namespace Alert {

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
				bool asyncronous = true;
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
			Activation(const Abstract::Alert *alert);
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
			inline Activation & set(const char *descr) noexcept {
				description = descr;
				return *this;
			}

			/// @brief Set level.
			inline Activation & set(const Udjat::Level level) noexcept {
				options.level = level;
				return *this;
			}

			/// @brief Set object (expand ${} on strings).
			virtual Activation & set(const Abstract::Object &object);

			/// @brief Set objects (expand ${} on strings).
			Activation & apply(const Abstract::Object *object, ...) __attribute__ ((sentinel));

			/// @brief Expand activation strings.
			virtual Activation & expand(const std::function<bool(const char *key, std::string &value)> &expander);

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


	}



 }
