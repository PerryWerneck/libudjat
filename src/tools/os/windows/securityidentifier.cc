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
  * @brief Implements security identifier.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/win32/security.h>
 #include <udjat/win32/exception.h>
 #include <system_error>
 #include <sddl.h>

 using namespace std;

 namespace Udjat {

	Win32::SecurityIdentifier::SecurityIdentifier(WELL_KNOWN_SID_TYPE type) {

		DWORD szNeeded = SECURITY_MAX_SID_SIZE;
		sid = (PSID) LocalAlloc(LPTR, szNeeded);

		if(CreateWellKnownSid(type, NULL, sid, &szNeeded) == 0) {
			LocalFree(sid);
			throw Win32::Exception("Cant create well known SID");
		}

	}

	Win32::SecurityIdentifier::SecurityIdentifier(SID_IDENTIFIER_AUTHORITY identifier, const std::vector<DWORD> &dwSubAuthority) {

		this->sid = 0;

		if(dwSubAuthority.size() > 7) {
			throw std::system_error(EINVAL, std::system_category(),"More than 7 sub authorities");
		}

		DWORD dwSubAuth[8];
		memset(&dwSubAuth,0,sizeof(dwSubAuthority));
		for(size_t f=0;f<dwSubAuthority.size();f++) {
			dwSubAuth[f] = dwSubAuthority[f];
		}

		// Obtenho SID
		SID_IDENTIFIER_AUTHORITY Auth;
		Auth = identifier;
		PSID ps;
		if(!AllocateAndInitializeSid(&Auth, dwSubAuthority.size(), dwSubAuth[0], dwSubAuth[1], dwSubAuth[2], dwSubAuth[3], dwSubAuth[4], dwSubAuth[5], dwSubAuth[6], dwSubAuth[7], &ps)) {
			throw Exception("Cant allocated SID");
		}

		set(ps);

		FreeSid(ps);

	}

	Win32::SecurityIdentifier::~SecurityIdentifier() {
		LocalFree(sid);
	}

	void Win32::SecurityIdentifier::set(PSID sid) {

		if(this->sid) {
			LocalFree(this->sid);
		}

		DWORD szNeeded = SECURITY_MAX_SID_SIZE;
		this->sid = (PSID) LocalAlloc(LPTR, szNeeded);

		if(!CopySid(szNeeded,this->sid,sid)) {
			throw Exception("Cant copy SID");
		}

	}

	std::string Win32::SecurityIdentifier::to_string() const {
		LPSTR text = NULL;

		if(!ConvertSidToStringSid(sid,&text)) {
			throw Exception("Cant convert SID to string");
		}

		std::string rc((const char *) text);
		LocalFree(text);

		return rc;
	}

 }

