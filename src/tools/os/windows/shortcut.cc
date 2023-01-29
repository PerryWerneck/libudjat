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
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <private/win32.h>
 #include <winnls.h>
 #include <shobjidl.h>
 #include <objbase.h>
 #include <objidl.h>
 #include <shlguid.h>

 using namespace std;

 static HRESULT CreateShortCut(const char * pszTargetfile, const char * pszTargetargs, const char * pszLinkfile, const char * pszDescription, int iShowmode, const char * pszCurdir, LPSTR pszIconfile, int iIconindex) {

	// https://www.codeproject.com/Articles/11467/How-to-create-short-cuts-link-files

	IShellLink*   pShellLink;            // IShellLink object pointer
	IPersistFile* pPersistFile;          // IPersistFile object pointer
	wchar_t       wszLinkfile[MAX_PATH]; // pszLinkfile as Unicode string

	HRESULT hRes =
		CoCreateInstance(
			CLSID_ShellLink,			// predefined CLSID of the IShellLink object
			NULL,						// pointer to parent interface if part of aggregate
			CLSCTX_INPROC_SERVER,		// caller and called code are in same process
			IID_IShellLink,				// predefined interface of the IShellLink object
			(void **) &pShellLink);		// Returns a pointer to the IShellLink object

	if(!SUCCEEDED(hRes)) {
		return hRes;
	}

	if(pszTargetfile && *pszTargetfile) {
		hRes = pShellLink->SetPath(pszTargetfile);
	} else {
		char filename[MAX_PATH+1];
		memset(filename,0,MAX_PATH+1);
		GetModuleFileName(NULL,filename,MAX_PATH);
		hRes = pShellLink->SetPath(filename);
	}

	if(pszTargetargs) {
		hRes = pShellLink->SetArguments(pszTargetargs);
	} else {
		hRes = pShellLink->SetArguments("");
	}

	hRes = pShellLink->SetDescription(pszDescription);

	if(iShowmode > 0) {
		hRes = pShellLink->SetShowCmd(iShowmode);
	}

	if(pszCurdir && *pszCurdir) {
		hRes = pShellLink->SetWorkingDirectory(pszCurdir);
	} else {
		// g_autofree gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		hRes = pShellLink->SetWorkingDirectory(Udjat::Application::Path().c_str());
	}

	if(pszIconfile && *pszIconfile && iIconindex >= 0) {
		hRes = pShellLink->SetIconLocation(pszIconfile, iIconindex);
	}

	// Use the IPersistFile object to save the shell link
	hRes = pShellLink->QueryInterface(
			   IID_IPersistFile,			// pre-defined interface of the IPersistFile object
			   (void **) &pPersistFile);	// returns a pointer to the IPersistFile object


	if(SUCCEEDED(hRes)) {
		MultiByteToWideChar(CP_ACP, 0, pszLinkfile, -1, wszLinkfile, MAX_PATH);
		hRes = pPersistFile->Save(wszLinkfile, TRUE);
		pPersistFile->Release();
	}

	pShellLink->Release();

	return hRes;
 }

 namespace Udjat {

	int SystemService::install_shortcut(const char UDJAT_UNUSED(*id), const char *name, const char *description, const char *arguments, bool autostart) {

		CreateShortCut(
			NULL,			// pszTargetfile
			arguments,		// pszTargetargs
			name,			// pszLinkfile
			description,	// pszDescription
			0,				// iShowmode
			NULL,			// pszCurdir
			NULL,			// pszIconfile
			0				// iIconindex
		);

		if(!autostart) {
			return 0;
		}

		// Create another shortcut in autostart (FOLDERID_CommonStartup)
		win32_special_folder(FOLDERID_CommonStartup);

		return 0;
	}

 }

