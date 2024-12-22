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

 #include <config.h>
 #include <udjat/alert.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/action.h>
 #include <stdexcept>

 #define LOG_DOMAIN "alert"
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	static Container<Alert::Factory> & Factories() {
		static Container<Alert::Factory> instance;
		return instance;
	}

	Alert::Factory::Factory(const char *name) : factory_name{name} {
		Factories().push_back(this);
	}

	Alert::Factory::~Factory() {
		Factories().remove(this);
	}

	std::shared_ptr<Alert> Alert::Factory::AlertFactory(const Abstract::Object &parent, const XML::Node &node) const {
		return std::shared_ptr<Alert>();
	}

	std::shared_ptr<Alert> Alert::Factory::build(const Abstract::Object &parent, const XML::Node &node) {

		class ActionAlert : public Alert {
		private:
			std::vector<std::shared_ptr<Action>> actions;

		public:
			ActionAlert(const XML::Node &node, std::shared_ptr<Action> action) : Alert{node} {
				if(!action) {
					throw runtime_error("Action alert requires an action");
				}
				actions.push_back(action);
			}

			ActionAlert(const XML::Node &node, std::shared_ptr<Action> action) : Alert{node} {
				Action::load(node,actions);
				if(!actions.empty()) {
					throw runtime_error("Action alert requires an action");
				}
			}

			int emit() override {
				for(auto action : actions) {
					int rc = action->call();
					if(rc) {
						return rc;
					}
				}
				return 0;
			}

		};

		if(!String{node,"action-type"}.empty()) {
			return make_shared<ActionAlert>(node,Action::Factory::build(node,"action-type",true));
		}

		String type{node,"type","default"};

		for(Alert::Factory *factory : Factories()) {

			if(*factory == type.c_str()) {

				try {

					std::shared_ptr<Alert> alert = factory->AlertFactory(parent,node);

					if(alert) {
						return alert;
					}

				} catch(const std::exception &e) {

					Logger::String{e.what()}.error(factory->name());

				} catch(...) {

					Logger::String{"Unexpected error building alert"}.error(factory->name());

				}

			}

		}

		return make_shared<ActionAlert>(node);

	}

 }

