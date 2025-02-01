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
#include <udjat/tools/url.h>

namespace Udjat {

	/// @brief Handle File:// URL.
	class UDJAT_PRIVATE FileURLHandler : public URL::Handler {	
	public:
		FileURLHandler(const URL &url) : URL::Handler{url} {
		}

		int perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const char *data, size_t len)> &progress) override;

		int test(const HTTP::Method method = HTTP::Head, const char *payload = "") override;


	};

	/// @brief Handle script:// URL.
	class UDJAT_PRIVATE ScriptURLHandler : public URL::Handler {	
	public:
		ScriptURLHandler(const URL &url) : URL::Handler{url} {
		}

		int perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const char *data, size_t len)> &progress) override;

		int test(const HTTP::Method method = HTTP::Head, const char *payload = "") override;


	};

}
