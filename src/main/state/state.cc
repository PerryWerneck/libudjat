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

/**
 * @file src/core/state/state.cc
 *
 * @brief Implements the abstract state methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/alert.h>
 #include <iostream>
 #include <udjat/tools/timestamp.h>
 #include <udjat.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::State::State(const Level level, const char *summary, const char *body)
		:  State(levelnames[level],level,Quark(summary).c_str(),Quark(body).c_str()) { }

	Abstract::State::State(const pugi::xml_node &node) : State(Attribute(node,"name",false).c_str()) {
		set(node);
	}

	void Abstract::State::set(const pugi::xml_node &node) {

		auto level = Attribute(node,"level",false);

		if(!(name && *name)) {
			name = level.c_str();
		}

		this->level = getLevelFromName(level.as_string(levelnames[unimportant])),
		this->summary = Attribute(node,"summary",true).c_str();
		this->name = Attribute(node,"name").c_str();
		this->uri = Attribute(node,"uri").c_str();

	}

	Abstract::State::~State() {

	}

	void Abstract::State::get(Udjat::Value &value) const {

		value["summary"] = summary;
		value["body"] = body;
		value["uri"] = uri;

		// Set level information
		getLevel(value);

	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::State::get(const Request &request, Response &response) const {
//		this->get(response);
	}
	#pragma GCC diagnostic pop


	void Abstract::State::activate(const Agent &agent) noexcept {

#ifdef DEBUG
		agent.info("State '{}' was activated",name);
#endif // DEBUG
	}

	void Abstract::State::deactivate(const Agent &agent) noexcept {
#ifdef DEBUG
		agent.info("State '{}' was deactivated",name);
#endif // DEBUG
	}

	void Abstract::State::expand(std::string &text) const {

		Udjat::expand(text,[this](const char *key) {

			if(!strcasecmp(key,"level")) {
				return string(to_string(this->level));
			}

			struct {
				const char *key;
				const Quark &value;
			} values[] = {
				{ "state.name",		this->name		},
				{ "state.summary",	this->summary	},
				{ "state.body",		this->body		},
				{ "state.uri", 		this->uri		},
			};

			for(size_t ix = 0; ix < (sizeof(values)/sizeof(values[0]));ix++) {

				if(!strcasecmp(values[ix].key,key)) {
					return string(values[ix].value.c_str());
				}

			}

			return string{"${}"};

		});

	}

	std::shared_ptr<Abstract::State> Abstract::State::get(const char *summary, const std::exception &e) {

		const std::system_error *syserror = dynamic_cast<const std::system_error *>(&e);

		if(!syserror) {

			// It's regular error
			class Error : public Abstract::State {
			private:
				string summary;
				string body;

			public:
				Error(const char *s, const exception &e) : Abstract::State("error",Udjat::critical), summary(s), body(e.what()) {
					Abstract::State::body = this->body.c_str();
					Abstract::State::summary = this->summary.c_str();
				}

			};

			return make_shared<Error>(summary,e);

		}

		/// @brief System Error State
		class SysError : public Udjat::State<int> {
		private:
			string summary;
			string body;

		public:
			SysError(const char *s, const system_error *e) : Udjat::State<int>("error",e->code().value(),Udjat::critical),summary(s),body(e->what()) {
				Abstract::State::body = this->body.c_str();
				Abstract::State::summary = this->summary.c_str();
			}

		};

		// It's a system error, create state from it.
		// id = code.value()
		// body = code.message();
		return make_shared<SysError>(summary,syserror);

	}

}
