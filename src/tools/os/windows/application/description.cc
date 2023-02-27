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
 #include <winver.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <udjat/tools/application.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/charset.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 namespace Udjat {

	Application::Description::Description() {

		TCHAR pszTargetfile[MAX_PATH];
		if(!GetModuleFileName(NULL, pszTargetfile, MAX_PATH ) ) {
			throw runtime_error("Can't get application filename");
		}

		DWORD szVersion = GetFileVersionInfoSize(pszTargetfile,NULL);
		if(!szVersion) {
			throw Win32::Exception("GetFileVersionInfoSize");
		}

		uint8_t pBlock[szVersion+1];
		memset(pBlock,0,szVersion+1);

		if(GetFileVersionInfo(pszTargetfile,0,szVersion,pBlock) == 0) {
			throw Win32::Exception("GetFileVersionInfo");
		}

		/*
		struct LANGANDCODEPAGE {
			WORD wLanguage;
			WORD wCodePage;
		} *lpTranslate;

		UINT cbTranslate = 0;
		VerQueryValue(pBlock,TEXT("\\VarFileInfo\\Translation"),(LPVOID*)&lpTranslate,&cbTranslate);

		for(UINT i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ ) {
			printf("\\StringFileInfo\\%04x%04x\\FileDescription\n",lpTranslate[i].wLanguage,lpTranslate[i].wCodePage);
		}
		*/

		const char *description = nullptr;
		UINT dwBytes;

		// Try translated version
		if(VerQueryValue(pBlock,TEXT(_("\\StringFileInfo\\080904B0\\FileDescription")),(LPVOID *) &description,&dwBytes)) {
			assign(description);
			return;
		}

		// Try English version
		if(VerQueryValue(pBlock,TEXT("\\StringFileInfo\\080904B0\\FileDescription"),(LPVOID *) &description,&dwBytes)) {
			assign(description);
			return;
		}

		throw system_error(ENODATA,system_category(),"Unable to get application description");

	}

 }
