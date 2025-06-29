/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <private/event.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/service.h>
 #include <udjat/module/abstract.h>
 #include <udjat/module/abstract.h>

 using namespace std;

 namespace Udjat {

	/*
	int Application::deinit() {
		ThreadPool::getInstance().wait();

		Service::for_each([](const Service &service){
			if(service.active()) {
				const_cast<Service *>(&service)->stop();
			}
			return true;
		});

		Module::for_each([](Module &module){
			module.finalize();
			return false;
		});

		ThreadPool::getInstance().wait();

		Module::unload();

		return 0;
	}
	*/

 }


