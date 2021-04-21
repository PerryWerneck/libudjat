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
 #include <iostream>

 namespace Udjat {

	/// @brief Default Alert Factory.
	class AlertFactory : public Factory {
	public:
		AlertFactory() : Factory(Quark::getFromStatic("alert")) {

			static const Udjat::ModuleInfo info{
				PACKAGE_NAME,								// The module name.
				"Default alert Factory",					// The module description.
				PACKAGE_VERSION "." PACKAGE_RELEASE,		// The module version.
#ifdef PACKAGE_URL
				PACKAGE_URL,
#else
				"",
#endif // PACKAGE_URL
#ifdef PACKAGE_BUG_REPORT
				PACKAGE_BUG_REPORT
#else
				""
#endif // PACKAGE_BUG_REPORT
			};

			this->info = &info;

		}

		virtual ~AlertFactory() {
		}

		void init() const {
			cout << "alert\t" << "The default alert type is '"
					<< Config::Value<string>("alert-default","type","url")
					<< "'" << endl;
		}

		const string getFactoryNameByType(const pugi::xml_node &node) const {

			return string{"alert-"}
				+ Attribute(node,"type")
					.as_string(
						Config::Value<string>(
							Alert::getConfigSection(node).c_str(),
							"type","url").c_str()
					);

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

	/// @brief URL Alert Factory.
	class URLAlertFactory : public Factory {
	public:
		URLAlertFactory() : Factory(Quark::getFromStatic("alert-url")) {

			static const Udjat::ModuleInfo info{
				PACKAGE_NAME,								// The module name.
				"URL Based alert Factory",					// The module description.
				PACKAGE_VERSION "." PACKAGE_RELEASE,		// The module version.
#ifdef PACKAGE_URL
				PACKAGE_URL,
#else
				"",
#endif // PACKAGE_URL
#ifdef PACKAGE_BUG_REPORT
				PACKAGE_BUG_REPORT
#else
				""
#endif // PACKAGE_BUG_REPORT
			};

			this->info = &info;

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

	void Alert::init() {

		static const struct {
			AlertFactory def;
			URLAlertFactory url;
		} factories;

		factories.def.init();
	}

 }
