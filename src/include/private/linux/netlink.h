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

 #pragma once
 
 #include <config.h>
 #include <udjat/defs.h>
 #include <sys/socket.h>
 #include <system_error>
 #include <unistd.h>
 #include <functional>
 #include <linux/rtnetlink.h>

 using namespace std;

 /// @brief Simple socket wrapper.
 /// @details This class is used handle a socket.
 struct UDJAT_PRIVATE Socket {

	int fd;

	Socket(int domain, int type, int protocol) : fd{socket(domain,type,protocol)} {
		if(fd < 0) {
			throw system_error(errno,system_category(),"Cant get socket");
		}
	}

	~Socket() {
		::close(fd);
	}

	inline operator int() const noexcept {
		return fd;
	}

 };

 UDJAT_PRIVATE bool netlink_routes(const std::function<bool(const struct rtattr *rtAttr)> &func);
