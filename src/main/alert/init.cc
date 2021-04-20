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

 #include "private.h"
 #include <udjat/url.h>
 #include <udjat/factory.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>

 namespace Udjat {

	/*
	class URLAlert : public Udjat::Alert {
	private:

		/// @brief The URL request method.
		URL::Method method;

		/// @brief The URL.
		Quark url;

		/// @brief Connection timeout.
		time_t timeout = 60;

		/// @brief Mimetype.
		string mimetype = "application/json; charset=utf-8";

	public:

		URLAlert(const pugi::xml_node &node) : Alert(node) {

			string section = getConfigSection(node);

			method =
				Attribute(node,"method",false)
					.as_string(
						Config::Value<string>(section.c_str(),"method",method.c_str()).c_str()
					);

			url =
				Attribute(node,"url",false)
					.as_string(
						Config::Value<string>(section.c_str(),"url",url.c_str()).c_str()
					);

			timeout =
				Attribute(node,"timeout")
					.as_uint(
						Config::Value<unsigned int>(section.c_str(),"timeout",timeout)
					);

			mimetype =
				Attribute(node,"mime-type",false)
					.as_string(
						Config::Value<string>(section.c_str(),"mime-type",mimetype.c_str()).c_str()
					);

		}

		virtual ~URLAlert() {
		}

	};

	class URLAlertFactory : public Factory {
	public:

		URLAlertFactory() : Factory(Quark::getFromStatic("alert-url")) {
			this->info = &module_info;
		}

		virtual ~URLAlertFactory() {
		}

 		void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {
 			parent.push_back(make_shared<URLAlert>(node));
 		}

		void parse(Abstract::State &parent, const pugi::xml_node &node) const override {
 			parent.push_back(make_shared<URLAlert>(node));
 		}

	};

	class AlertFactory : public Factory {
	private:

	public:
		AlertFactory() : Factory(Quark::getFromStatic("alert")) {
			this->info = &module_info;
		}

		virtual ~AlertFactory() {
		}

 		void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {
 			Factory::parse(
				getFactoryNameByType(node).c_str(),
				parent,
				node
			);
 		}

		void parse(Abstract::State &parent, const pugi::xml_node &node) const override {
			Factory::parse(
				getFactoryNameByType(node).c_str(),
				parent,
				node
			);
 		}

	};
	*/

	void Alert::init() {
	}

 }
