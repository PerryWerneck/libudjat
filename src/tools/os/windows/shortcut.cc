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
 #include <udjat/win32/exception.h>
 #include <comdef.h>

 using namespace std;
 using namespace Udjat;

 static void CreateShortCut(const char * pszTargetargs, const char * pszLinkfile, const char * pszDescription, int iShowmode, const char * pszCurdir, LPSTR pszIconfile, int iIconindex) {

	TCHAR pszTargetfile[MAX_PATH];

	if(!GetModuleFileName(NULL, pszTargetfile, MAX_PATH ) ) {
		throw runtime_error("Can't get application filename");
	}

	debug("Creating shortcut for '",pszTargetfile,"'");

	// https://www.codeproject.com/Articles/11467/How-to-create-short-cuts-link-files

	// TODO: CComPtr should be better here but where is it defined?

	IShellLink * pShellLink = NULL;		// IShellLink object pointer
	Win32::throw_if_fail(
		CoCreateInstance(
			CLSID_ShellLink,			// predefined CLSID of the IShellLink object
			NULL,						// pointer to parent interface if part of aggregate
			CLSCTX_INPROC_SERVER,		// caller and called code are in same process
			IID_IShellLink,				// predefined interface of the IShellLink object
			(void **) &pShellLink		// Returns a pointer to the IShellLink object
		)
	);

	try {

		Win32::throw_if_fail(pShellLink->SetPath(pszTargetfile));

		if(pszTargetargs && *pszTargetargs) {
			Win32::throw_if_fail(pShellLink->SetArguments(pszTargetargs));
		}

		if(pszDescription && *pszDescription) {
			Win32::throw_if_fail(pShellLink->SetDescription(pszDescription));
		}

		if(iShowmode > 0) {
			Win32::throw_if_fail(pShellLink->SetShowCmd(iShowmode));
		}

		if(pszCurdir && *pszCurdir) {

			Win32::throw_if_fail(pShellLink->SetWorkingDirectory(pszCurdir));

		} else {

			Win32::throw_if_fail(pShellLink->SetWorkingDirectory(Udjat::Application::Path().c_str()));

		}

		if(pszIconfile && *pszIconfile && iIconindex >= 0) {
			Win32::throw_if_fail(pShellLink->SetIconLocation(pszIconfile, iIconindex));
		}

		// Use the IPersistFile object to save the shell link
		IPersistFile *pPersistFile = NULL;          // IPersistFile object pointer
		Win32::throw_if_fail(
				pShellLink->QueryInterface(
				   IID_IPersistFile,			// pre-defined interface of the IPersistFile object
				   (void **) &pPersistFile)		// returns a pointer to the IPersistFile object
		);

		wchar_t wszLinkfile[MAX_PATH+1]; // pszLinkfile as Unicode string
		MultiByteToWideChar(CP_ACP, 0, pszLinkfile, -1, wszLinkfile, MAX_PATH);

		try {

			Win32::throw_if_fail(pPersistFile->Save(wszLinkfile, TRUE));

		} catch(...) {

			pPersistFile->Release();
			throw;

		}

		pPersistFile->Release();

	} catch(...) {

		pShellLink->Release();
		throw;

	}

	pShellLink->Release();

 }

 namespace Udjat {

	void Application::shortcut(const char UDJAT_UNUSED(*id), const char *description, const char *arguments) {

		CreateShortCut(
			arguments,										// pszTargetargs
			(string{".\\"} + Name() + ".lnk").c_str(),		// pszLinkfile
			description,									// pszDescription
			0,												// iShowmode
			NULL,											// pszCurdir
			NULL,											// pszIconfile
			0												// iIconindex
		);


		/*
		if(!autostart) {
			return 0;
		}

		// Create another shortcut in autostart (FOLDERID_CommonStartup)
		CreateShortCut(
			(win32_special_folder(FOLDERID_CommonStartup) + Application::Name()).c_str(),			// pszTargetfile
			arguments,		// pszTargetargs
			name,			// pszLinkfile
			description,	// pszDescription
			0,				// iShowmode
			NULL,			// pszCurdir
			NULL,			// pszIconfile
			0				// iIconindex
		);
		*/

	}

	void Application::autostart(const char UDJAT_UNUSED(*id), const char *comment, const char *arguments) {
	}

 }

