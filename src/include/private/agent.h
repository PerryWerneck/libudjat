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
#include <udjat/agent.h>
#include <udjat/tools/mainloop.h>
#include <udjat/tools/service.h>
#include <udjat/tools/timer.h>
#include <udjat/action.h>
#include <udjat/tools/interface.h>
#include <memory>

namespace Udjat {

	class Abstract::Agent::Controller : private Service, public MainLoop::Timer, private Abstract::Object::Factory, private Interface {
	private:

		time_t updating = 0;

		std::shared_ptr<Abstract::Agent> root;

		Controller(const Controller &) = delete;
		Controller(const Controller *) = delete;
		Controller();

		void on_timer() override;

		void update_agents();

		bool schema(const HTTP::Method method, const char *path, Schema::Output &s) const noexcept override;
		bool schema(const HTTP::Method method, const char *path, Schema::Input &s) const noexcept override;
		bool schema(const char *path, Schema::Method &schema) const noexcept override;

	public:
		~Controller();

		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}

		void set(std::shared_ptr<Abstract::Agent> root);
		std::shared_ptr<Abstract::Agent> get() const;
		
		std::shared_ptr<Abstract::Agent> find(const char *path, bool required = false) const;

		void start() noexcept override;
		void stop() noexcept override;

		// Object Factory.
		std::shared_ptr<Abstract::Object> ObjectFactory(const Udjat::Properties &props) const override;
	
		// Interface
		bool process(Request &request, Response &response) const noexcept override;
		bool get_property(const char *path, const char *name, Variant &value) const override;

	};

}
