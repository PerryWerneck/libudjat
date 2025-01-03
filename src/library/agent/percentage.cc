/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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

/**
 *
 * @brief Implements the percentage agent.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/agent/percentage.h>
 #include <udjat/agent/state.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/intl.h>
 #include <memory>
 #include <sstream>
 #include <iomanip>

 using namespace std;

 namespace Udjat {

	Percentage::Percentage(const char *name, const char *label, const char *summary) : Udjat::Agent<float>(name) {

		if(label && *label) {
			Object::properties.label = label;
		}

		if(summary && *summary) {
			Object::properties.summary = summary;
		}
	}

	static bool empty_str(const char *str) {
		return !(str && *str);
	}

	Percentage::Percentage(const pugi::xml_node &node, const char *label, const char *summary) : Udjat::Agent<float>(node) {

		if(empty_str(Object::properties.label) && label && *label) {	
			Object::properties.label = label;
		}

		if(empty_str(Object::properties.summary) && summary && *summary) {	
			Object::properties.summary = summary;
		}

	}

	Percentage::~Percentage() {
	}

	std::string Percentage::to_string() const noexcept {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << (get() * 100) << "%";
		return stream.str();
	}

	void Percentage::append(const StateDescription &state, float &value) {

		this->push_back(
			make_shared<State<float>>(
				state.name,
				value,
				state.value,
				state.level,
				state.summary,
				state.body
			)
		);

		value = state.value;

	}

	std::shared_ptr<Abstract::State> Percentage::StateFactory(const pugi::xml_node &node) {

		class State : public Udjat::State<float> {
		public:
			State(const pugi::xml_node &node) : Udjat::State<float>(node) {
				from /= 100;
				to /= 100;
			}

		};

		auto state = std::make_shared<State>(node);
		states.push_back(state);
		return state;

	}

 }

