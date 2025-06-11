/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 * @brief LibUdjat test methods.
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 */

#pragma once

#include <udjat/defs.h>
#include <udjat/tools/systemservice.h>
#include <udjat/tools/application.h>
#include <functional>

namespace Udjat {

	namespace Testing {

		int run(int argc, char **argv, const Udjat::ModuleInfo &info, const char *xml = "./test.xml");
		int run(int argc, char **argv, const Udjat::ModuleInfo &info, const std::function<void(Udjat::Application &app)> &initialize, const char *xml = "./test.xml");

		/// @brief Test agent, reports a random unsignet int value
		class UDJAT_PRIVATE RandomFactory : public Udjat::Abstract::Agent::Factory {
		public:
			RandomFactory(const Udjat::ModuleInfo &info);
			std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Agent &parent, const XML::Node &node) const = 0;

		};

		/// @brief System service tester.
		class UDJAT_PRIVATE Service : public SystemService, private RandomFactory {
		public:
			Service(int argc, char **argv, const Udjat::ModuleInfo &info);
			Service(int argc, char **argv, const Udjat::ModuleInfo &info, const std::function<void(Udjat::Application &app)> &initialize);
			void root(std::shared_ptr<Abstract::Agent> agent);

			static int run_tests(int argc, char **argv, const Udjat::ModuleInfo &info);
			
		};

		/// @brief Application tester.
		class Application : public Udjat::Application, private RandomFactory {
		public:
			Application(int argc, char **argv, const Udjat::ModuleInfo &info);
			Application(int argc, char **argv, const Udjat::ModuleInfo &info, const std::function<void(Udjat::Application &app)> &initialize);
			void root(std::shared_ptr<Abstract::Agent>);

			int install(const char *name) override;
			int uninstall() override;

			static int run_tests(int argc, char **argv, const Udjat::ModuleInfo &info);

		};


	}


}