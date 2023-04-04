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
 #include <udjat/win32/charset.h>
 #include <udjat/win32/com.h>
 #include <comdef.h>

 using namespace std;
 using namespace Udjat;
 using namespace Win32;

 static void CreateShortCut(const Win32String & Linkfile, const Win32String & Description, const Win32String & Targetargs, int iShowmode, const char * pszCurdir, LPSTR pszIconfile, int iIconindex) {

	TCHAR pszTargetfile[MAX_PATH];
	if(!GetModuleFileName(NULL, pszTargetfile, MAX_PATH ) ) {
		throw runtime_error("Can't get application filename");
	}

	debug("Shortcut file is '",Linkfile.c_str(),"'");
	debug("Shortcut target file is '",pszTargetfile,"'");

	// https://www.codeproject.com/Articles/11467/How-to-create-short-cuts-link-files

	// TODO: CComPtr should be better here but where is it defined?
	Win32::Com::Instance<IShellLink> ishelllink{CLSID_ShellLink,CLSCTX_INPROC_SERVER,IID_IShellLink};

	/*
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
	*/

	Win32::throw_if_fail(ishelllink->SetPath(pszTargetfile));

	if(!Targetargs.empty()) {
		Win32::throw_if_fail(ishelllink->SetArguments(Targetargs.c_str()));
	}

	if(!Description.empty()) {
		Win32::throw_if_fail(ishelllink->SetDescription(Description.c_str()));
	}

	if(iShowmode > 0) {
		Win32::throw_if_fail(ishelllink->SetShowCmd(iShowmode));
	}

	if(pszCurdir && *pszCurdir) {

		Win32::throw_if_fail(ishelllink->SetWorkingDirectory(pszCurdir));

	} else {

		Win32::throw_if_fail(ishelllink->SetWorkingDirectory(Udjat::Application::Path().c_str()));

	}

	if(pszIconfile && *pszIconfile && iIconindex >= 0) {
		Win32::throw_if_fail(ishelllink->SetIconLocation(pszIconfile, iIconindex));
	}

	// Use the IPersistFile object to save the shell link
	Com::Object<IPersistFile> file;

	Win32::throw_if_fail(ishelllink->QueryInterface(IID_IPersistFile,file.ptr()));

	wchar_t wszLinkfile[MAX_PATH+1]; // pszLinkfile as Unicode string
	MultiByteToWideChar(CP_ACP, 0, Linkfile.c_str(), -1, wszLinkfile, MAX_PATH);

	Win32::throw_if_fail(file->Save(wszLinkfile, TRUE));

	SHChangeNotify(SHCNE_CREATE, SHCNF_PATH | SHCNF_FLUSHNOWAIT, Linkfile.c_str(), NULL);

 }

 namespace Udjat {

 	Application::ShortCut::ShortCut(const Type t, const char *i, const char *n, const char *c, const char *a)
		: type{t}, id{i}, name{n}, description{c}, arguments{a} {

		if(name.empty()) {
			name = Description();
		}

	}

	Application::ShortCut & Application::ShortCut::remove() {

		std::vector<Win32::KnownFolder> folders;

		folders.emplace_back(FOLDERID_CommonStartup);
		folders.emplace_back(FOLDERID_Desktop);

		switch(type) {
		case Generic:
			folders.emplace_back(FOLDERID_CommonPrograms);
			break;

		case AdminTool:
			folders.emplace_back(FOLDERID_CommonAdminTools);
			break;

		default:
			throw runtime_error("Unexpected application type");

		}

		for(auto &folder : folders) {

			std::string linkfile{folder.c_str()};
			linkfile += Win32String{name}.c_str();
			linkfile += ".lnk";

			if(DeleteFile(linkfile.c_str()) == 0) {
				auto err = GetLastError();
				if(err != ERROR_FILE_NOT_FOUND) {
					cerr << Win32::Exception::format(linkfile.c_str(),err) << endl;
				}
			} else {
				SHChangeNotify(SHCNE_DELETE, SHCNF_PATH | SHCNF_FLUSHNOWAIT, linkfile.c_str(), NULL);
			}

		}

		return *this;
	}

	Application::ShortCut & Application::ShortCut::save() {

		std::string linkfile;

		// https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
		switch(type) {
		case Generic:
			linkfile = Win32::KnownFolder{FOLDERID_CommonPrograms};
			break;

		case AdminTool:
			linkfile = Win32::KnownFolder{FOLDERID_CommonAdminTools};
			break;

		default:
			throw runtime_error("Unexpected application type");

		}

		linkfile += Win32String{name}.c_str();
		linkfile += ".lnk";

		CreateShortCut(
			linkfile,					// pszLinkfile
			Win32String{description},	// pszDescription
			Win32String{arguments},		// pszTargetargs
			0,							// iShowmode
			NULL,						// pszCurdir
			NULL,						// pszIconfile
			0							// iIconindex
		);

		return *this;
	}

	Application::ShortCut & Application::ShortCut::autostart() {

		Win32::KnownFolder linkfile{FOLDERID_CommonStartup};

		linkfile += name;
		linkfile += ".lnk";

		CreateShortCut(
			linkfile.c_str(),		// pszLinkfile
			description.c_str(),	// pszDescription
			arguments.c_str(),		// pszTargetargs
			0,						// iShowmode
			NULL,					// pszCurdir
			NULL,					// pszIconfile
			0						// iIconindex
		);

		return *this;
	}

	Application::ShortCut & Application::ShortCut::desktop() {
		Win32::KnownFolder linkfile{FOLDERID_Desktop};

		linkfile += name;
		linkfile += ".lnk";

		CreateShortCut(
			linkfile.c_str(),		// pszLinkfile
			description.c_str(),	// pszDescription
			arguments.c_str(),		// pszTargetargs
			0,						// iShowmode
			NULL,					// pszCurdir
			NULL,					// pszIconfile
			0						// iIconindex
		);

		return *this;
	}

	/*
	void Application::shortcut(const Application::Type type, const char UDJAT_UNUSED(*id), const char *name, const char *description, const char *arguments) {


	}

	void Application::autostart(const Application::Type type, const char UDJAT_UNUSED(*id), const char *name, const char *description, const char *arguments) {


	}
	*/

 }

