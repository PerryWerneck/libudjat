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
 * @file main/load/xml.cc
 *
 * @brief Implements the application settings loader.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/misc.h>
 #include <sys/stat.h>
 #include <udjat/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/timer.h>
 #include <udjat/module.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <private/updater.h>
 #include <list>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	time_t UDJAT_API Application::setup(const char *pathname, bool force) {

		Updater updater{pathname,force};

		if(updater.refresh()) {
			updater.load(RootAgentFactory());
		}

		return updater.wait();
	}

 }
