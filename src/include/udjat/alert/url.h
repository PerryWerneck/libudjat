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
 #include <udjat/alert/abstract.h>
 #include <udjat/alert/activation.h>

 namespace Udjat {

	namespace Alert {

		/// @brief Default alert (based on URL and payload).
		class UDJAT_API URL : public Abstract::Alert {
		protected:

			const char *url = "";
			HTTP::Method action = HTTP::Get;
			const char *payload = "";

			/// @brief URL based alert activation.
			class UDJAT_API Activation : public Udjat::Alert::Activation {
			protected:
				String url;
				HTTP::Method action;
				String payload;

			public:
				Activation(const Udjat::Alert::URL *alert);
				void emit() override;

				Value & getProperties(Value &value) const override;

				Udjat::Alert::Activation & set(const Abstract::Object &object) override;
				Udjat::Alert::Activation & set(const std::function<bool(const char *key, std::string &value)> &expander) override;

			};

			std::shared_ptr<Udjat::Alert::Activation> ActivationFactory() const override;

		public:

			constexpr URL(const char *name, const char *u, const HTTP::Method a = HTTP::Get, const char *p = "") : Abstract::Alert(name), url(u), action(a), payload(p) {
			}

			URL(const XML::Node &node, const char *defaults = "alert-defaults");

			/// @brief Get alert info.
			Value & getProperties(Value &value) const override;

		};

	}

 }
