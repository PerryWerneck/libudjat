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
#include <udjat/tools/url/handler.h>
#include <string>

#ifdef HAVE_SMBIOS
	#include <smbios/value.h>
#endif // HAVE_SMBIOS

namespace Udjat {

	/// @brief Handle File:// URL.
	class UDJAT_PRIVATE FileURLHandler : public URL::Handler {	
	private:
		String path;

	public:
		FileURLHandler(const URL &url) : path{url.path().c_str()} {
		}

		const char *c_str() const noexcept override;

		int perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) override;

		int test(const HTTP::Method method = HTTP::Head, const char *payload = "") override;


	};

	/// @brief Handle script:// URL.
	class UDJAT_PRIVATE ScriptURLHandler : public URL::Handler {	
	private:
		String path;

	public:
		ScriptURLHandler(const URL &url) : path{url.path().c_str()} {
		}

		const char *c_str() const noexcept override;

		int perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) override;

		int test(const HTTP::Method method = HTTP::Head, const char *payload = "") override;


	};

#ifdef HAVE_SMBIOS
	/// @brief Handle dmi:// URL or smbios:// URL.
	/// @details This handler is used to access DMI information on the system
	class UDJAT_PRIVATE SMBiosURLHandler : public URL::Handler {	
	private:
		URL url;
		MimeType mimetype = MimeType::json; ///< @brief The requested mimetype, default is 'application/json'.

	public:

		SMBiosURLHandler(const URL &url);

		const char *c_str() const noexcept override;

		bool get(Udjat::Value &value, const HTTP::Method method = HTTP::Get, const char *payload = "") override;

		int perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) override;

		int test(const HTTP::Method method = HTTP::Head, const char *payload = "") override;

		Handler & set(const MimeType mimetype) override;

	};
#endif

}
