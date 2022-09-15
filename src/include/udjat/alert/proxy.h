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
 #include <udjat/tools/xml.h>
 #include <udjat/alert.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/tools/parse.h>

 namespace Udjat {

	namespace Alert {

		/// @brief Alert proxy.
		template <typename T>
		class UDJAT_API Proxy {
		protected:
			std::shared_ptr<Abstract::Alert> alert;
			T value;

		public:
			Proxy(std::shared_ptr<Abstract::Alert> a, T v) : alert(a), value(t) {
			}

			Proxy(const XML::Node &node) {
				XML::parse(node, value);
			}

			Proxy(const XML::Node &node, std::shared_ptr<Abstract::Alert> a) : alert(a) {
				XML::parse(node, value);
			}

			inline bool operator ==(const T value) const noexcept {
				return this->value == value;
			}

			inline T operator &(const T value) const noexcept {
				return (T) (this->value & value);
			}

			inline operator T () const noexcept {
				return this->value;
			}

			/// @brief Get alert info.
			inline Value & getProperties(Value &value) const noexcept {
				return alert->getProperties(value);
			}

			inline std::shared_ptr<Udjat::Alert::Activation> ActivationFactory() const {
				return alert->ActivationFactory();
			}

			/// @brief Just emit alert, no update on emission data.
			inline void emit() {
				alert->emit();
			}

			/// @brief Deactivate an alert.
			inline void deactivate() {
				alert->deactivate();
			}

			inline std::ostream & info() const {
				return alert->info();
			}

			inline std::ostream & warning() const {
				return alert->warning();
			}

			inline std::ostream & error() const {
				return alert->error();
			}


		}

	}

 }
