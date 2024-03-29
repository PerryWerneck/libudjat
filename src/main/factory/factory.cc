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
 #include <udjat/defs.h>
 #include <private/factory.h>
 #include <udjat/agent.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/module/info.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	Factory::Factory(const char *n, const ModuleInfo &i) : factory_name(n), module(i) {
		Controller::getInstance().insert(this);
	}

	Factory::~Factory() {
		Controller::getInstance().remove(this);
	}

	Value & Factory::getProperties(Value &properties) const {
		properties["name"] = factory_name;
		return module.getProperties(properties);
	}

	bool Factory::for_each(const std::function<bool(Factory &factory)> &func) {
		return Controller::getInstance().for_each(func);
	}

	std::shared_ptr<Abstract::Agent> Factory::AgentFactory(const Abstract::Object UDJAT_UNUSED(&parent), const XML::Node UDJAT_UNUSED(&node)) const {
		return std::shared_ptr<Abstract::Agent>();
	}

	std::shared_ptr<Abstract::Object> Factory::ObjectFactory(const Abstract::Object UDJAT_UNUSED(&parent), const XML::Node UDJAT_UNUSED(&node)) const {
		return std::shared_ptr<Abstract::Object>();
	}

	std::shared_ptr<Abstract::Alert> Factory::AlertFactory(const Abstract::Object UDJAT_UNUSED(&parent), const XML::Node UDJAT_UNUSED(&node)) const {
		return std::shared_ptr<Abstract::Alert>();
	}

	std::shared_ptr<Activatable> Factory::ActivatableFactory(const Abstract::Object &parent, const XML::Node UDJAT_UNUSED(&node)) const {
		return AlertFactory(parent,node);
	}

	bool Factory::CustomFactory(const XML::Node &) {
		return false;
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	bool Factory::NodeFactory(const XML::Node &node) {
		return generic(node);
	}
	#pragma GCC diagnostic pop

	bool Factory::CustomFactory(Abstract::Object &, const XML::Node &node) {
		return CustomFactory(node);
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	bool Factory::NodeFactory(Abstract::Object &parent, const XML::Node &node) {
		return generic(parent,node);
	}
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	bool Factory::generic(Abstract::Object &, const XML::Node &node) {
		return generic(node);
	}
	#pragma GCC diagnostic pop

	bool Factory::generic(const XML::Node &) {
		return false;
	}

	std::ostream & Factory::info() const {
		return cout << name() << "\t";
	}

	std::ostream & Factory::warning() const {
		return clog << name() << "\t";
	}

	std::ostream & Factory::error() const {
		return cerr << name() << "\t";
	}

 }
