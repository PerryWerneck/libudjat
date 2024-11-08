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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tests.h>
 #include <udjat/tools/xml.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>

 using namespace std;

 namespace Udjat {

	int Testing::Service::run_tests(int argc, char **argv, const Udjat::ModuleInfo &info) {

		Config::allow_user_homedir(true);

		Logger::redirect();
		Logger::verbosity(9);
		Logger::console(true);

		/*
		class DummyProtocol : public Udjat::Protocol {
		public:
			DummyProtocol() : Udjat::Protocol("dummy",moduleinfo) {
			}

			String call(const URL &url, const HTTP::Method UDJAT_UNUSED(method), const char UDJAT_UNUSED(*payload)) const override {
				cout << "**** dummy\t[" << url << "]" << endl;
				return "";
			}

		};
		*/

		/*
		{
			SubProcess{"test","echo su - root --login -- /usr/sbin/grub2-reboot \"\\\"Reinstalar estação de trabalho\\\"\""}.run();
			return 0;
		}
		*/

		/*
		MainLoop::getInstance().TimerFactory(1000,[]{
			debug("---------------------------------------------------------------------");
			// SubProcess{"test","ls -l",Logger::Warning}.run();

			cout << URL{"agent:///intvalue"}.get() << endl;

			debug("---------------------------------------------------------------------");
			return false;
		});
		*/


		//DummyProtocol protocol;

		auto rc = Testing::Service{info}.run(argc,argv,"./test.xml");

		Logger::String{"Service exits with rc=",rc}.info("test");
		return rc;

	}

	Testing::Service::Service(const Udjat::ModuleInfo &info)
		: RandomFactory{info} {
	}

	Testing::Service::Service(const Udjat::ModuleInfo &info, const std::function<void()> &initialize)
		: Service{info} {
		initialize();
	}

	void Testing::Service::root(std::shared_ptr<Abstract::Agent> agent) {
		Logger::String{"The agent '",agent->name(),"' is the new root"}.info("service");
		SystemService::root(agent);
	}


 }

