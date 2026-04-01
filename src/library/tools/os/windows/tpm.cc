/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 #define LOG_DOMAIN "tpm"

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/crypto.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API bool TPM::probe(const bool except) {

		if(except) {
			throw system_error(ENOTSUP,system_category(),"TPM engine is not supported on windows");
		} else {
			Logger::String{"No TPM support on windows"}.warning();
		}

		return false;

	}


 }