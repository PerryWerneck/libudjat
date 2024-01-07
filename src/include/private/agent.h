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

#include <config.h>
#include <udjat/defs.h>
#include <iostream>
#include <udjat/agent.h>
#include <unordered_map>
#include <udjat/tools/quark.h>
#include <pugixml.hpp>
#include <udjat/tools/worker.h>
#include <udjat/module/abstract.h>
#include <udjat/factory.h>
#include <udjat/tools/mainloop.h>
#include <udjat/tools/service.h>
#include <udjat/tools/timer.h>
#include <udjat/tools/response.h>
#include <udjat/tools/report.h>
#include <udjat/tools/abstract/response.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif // HAVE_UNISTD_H

using namespace std;

namespace Udjat {

	class Abstract::Agent::Controller : private Worker, private Service, public MainLoop::Timer {
	private:

		time_t updating = 0;

		std::shared_ptr<Abstract::Agent> root;

		Controller(const Controller &) = delete;
		Controller(const Controller *) = delete;
		Controller();

		void on_timer() override;

		void update_agents();

	public:
		~Controller();

		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}

		void set(std::shared_ptr<Abstract::Agent> root);

		std::shared_ptr<Abstract::Agent> get() const;

		std::shared_ptr<Abstract::Agent> find(const char *path, bool required = false) const;

		/// @brief Find child, update path.
		/// @brief agent Pointer to root agent.
		/// @brief path pointer to child path.
		/// @return Pointer to child or nullptr.
		//static const Abstract::Agent * find(const Abstract::Agent *agent, const char * &path);

		/*
		/// @brief Get agent's cache properties.
		static bool head(Abstract::Agent *agent, const char *path, Abstract::Response &response);

		bool head(Request &request,  Udjat::Response::Value &response) const override;
		*/

		void start() noexcept override;
		void stop() noexcept override;

		/// @brief Load agent properties from XML node.
		static void setup_properties(Abstract::Agent &agent, const pugi::xml_node &node) noexcept;

		// Worker
		bool get(Request &request, Udjat::Response::Value &response) const override;
		bool get(Request &request, Udjat::Response::Table &response) const override;

	};

}
