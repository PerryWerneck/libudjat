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

/***
  * @brief Implement linux system methods.
  *
  */

#ifndef _WIN32
	#error Only for windows
#endif // _WIN32

 #include <config.h>
 #include <udjat/defs.h>
 #include <sysinfoapi.h>

 #include <memory>
 #include <udjat/tools/system.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API String System::cpe() {

		OSVERSIONINFO osvi;

		ZeroMemory(&osvi, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(osvi);

		GetVersionEx(&osvi);

		// https://en.wikipedia.org/wiki/Common_Platform_Enumeration

		// CPE 2.3 follows this format, maintained by NIST:[2]
		// cpe:<cpe_version>:<part>:<vendor>:<product>:<version>:<update>:<edition>:<language>:<sw_edition>:<target_sw>:<target_hw>:<other>

		// cpe_version
		// The version of the CPE definition. The latest CPE definition version is 2.3.

		// part
		// May have 1 of 3 values:

		// a for Applications
		// h for Hardware
		// o for Operating Systems
		// It is sometimes referred to as type.

		// vendor
		// Values for this attribute SHOULD describe or identify the person or organization that manufactured or created the product. Values for this attribute SHOULD be selected from an attribute-specific valid-values list, which MAY be defined by other specifications that utilize this specification. Any character string meeting the requirements for WFNs (cf. 5.3.2) MAY be specified as the value of the attribute. [1]

		// product
		// The name of the system/package/component. product and vendor are sometimes identical. It can not contain spaces, slashes, or most special characters. An underscore should be used in place of whitespace characters.

		// version
		// The version of the system/package/component.

		// update
		// This is used for update or service pack information. Sometimes referred to as "point releases" or minor versions. The technical difference between version and update will be different for certain vendors and products. Common examples include beta, update4, SP1, and ga (for General Availability), but it is most often left blank.

		// edition
		// A further granularity describing the build of the system/package/component, beyond version.

		// language
		// A valid language tag as defined by IETF RFC 4646 entitled "Tags for Identifying Languages". Examples include: en-us for US English

		// cpe:2.3:o:microsoft:windows_7:-:sp2:*:*:*:*:*:*
		
		return String{
			"cpe:2.3:o:Microsoft:Windows_",to_string(osvi.dwMajorVersion).c_str(),".",to_string(osvi.dwMinorVersion).c_str(),
			":-:",(osvi.szCSDVersion[0] ? (char *) osvi.szCSDVersion : to_string(osvi.dwBuildNumber).c_str()), // update
			":*:*:*:*:*:*"
		};

		return "";

	}

 }

