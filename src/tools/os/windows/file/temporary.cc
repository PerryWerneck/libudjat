/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 //
 // References:
 //
 // https://docs.microsoft.com/en-us/windows/win32/fileio/creating-and-using-a-temporary-file
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/file.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/application.h>
 #include <cstdio>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <iostream>
 #include <udjat/win32/path.h>

 #include <windows.h>
 #include <fileapi.h>
 #include <pathcch.h>

 using namespace std;

 namespace Udjat {

	std::string File::Temporary::create() {

		std::string tempname;

		for(size_t f = 0; f < 1000; f++) {

			TCHAR szTempFileName[MAX_PATH];
			TCHAR lpTempPathBuffer[MAX_PATH];

			DWORD dwRetVal = GetTempPath(MAX_PATH,lpTempPathBuffer);
			if(dwRetVal > MAX_PATH || (dwRetVal == 0)) {
				throw Win32::Exception("GetTempPath has failed");
			}

			if(GetTempFileName(lpTempPathBuffer,TEXT(Application::Name().c_str()),0,szTempFileName) == 0) {
				throw Win32::Exception("GetTempFileName has failed");
			}

			int fd = open(szTempFileName,O_CREAT|O_EXCL,0600);

			if(fd > 0) {
#ifdef DEBUG
				cout << "Tempname: '" << tempname << "'" << endl;
#endif // DEBUG
				::close(fd);
				return szTempFileName;
			}

		}

		throw runtime_error("Can't create temporary file");

	}

	File::Temporary::Temporary() {

		TCHAR szTempFileName[MAX_PATH];
		TCHAR lpTempPathBuffer[MAX_PATH];

		DWORD dwRetVal = GetTempPath(MAX_PATH,lpTempPathBuffer);
		if(dwRetVal > MAX_PATH || (dwRetVal == 0)) {
			throw Win32::Exception("GetTempPath has failed");
		}

		if(GetTempFileName(lpTempPathBuffer,TEXT(Application::Name().c_str()),0,szTempFileName) == 0) {
			throw Win32::Exception("GetTempFileName has failed");
		}

		tempname = szTempFileName;

		debug("Tempname: '",tempname,"'");

		fd = open(tempname.c_str(),O_TRUNC|O_RDWR|O_CREAT,0644);

		if(fd < 0) {
			throw system_error(errno,system_category(),"Unable to create and open temporary file");
		}

	}

	File::Temporary::Temporary(const char *name) : filename{Win32::Path{name}} {

		TCHAR lpTempPathBuffer[PATH_MAX+1];
		memset(lpTempPathBuffer,0,sizeof(lpTempPathBuffer));

		strncpy(lpTempPathBuffer,filename.c_str(),PATH_MAX);
		{
			char *ptr = strrchr(lpTempPathBuffer,'\\');
			if(ptr) {
				*(ptr+1) = 0;
			}
		}

		TCHAR szTempFileName[MAX_PATH];

		if(GetTempFileName(lpTempPathBuffer,TEXT(Application::Name().c_str()),0,szTempFileName) == 0) {
			throw Win32::Exception("GetTempFileName has failed");
		}

		tempname = szTempFileName;

		fd = open(tempname.c_str(),O_TRUNC|O_RDWR|O_CREAT,0644);

		if(fd < 0) {
			throw system_error(errno,system_category(),"Unable to create and open temporary file");
		}

	}

	File::Temporary::~Temporary() {
		if(fd > 0) {
			::close(fd);
			DeleteFile(tempname.c_str());
		}
	}

	void File::Temporary::save(const char *filename, bool replace) {

		if(fd > 0) {
			::close(fd);
			fd = -1;
		}

		if(replace) {
			DeleteFile(filename);
		}

		if(MoveFile(tempname.c_str(),filename)) {
			return;
		}

		if(replace || GetLastError() != ERROR_ALREADY_EXISTS) {
			throw Win32::Exception("Can't save file");
		}

		char bakfile[PATH_MAX+1];
		strncpy(bakfile,filename,PATH_MAX);
		char *ptr = strrchr(bakfile,'.');
		if(ptr) {
			*ptr = 0;
		}
		strncat(bakfile,".bak",PATH_MAX);

		DeleteFile(bakfile);
		MoveFile(filename,bakfile);

		if(MoveFile(tempname.c_str(),filename)) {
			return;
		}

		throw Win32::Exception("Can't save file");

	}


	void File::Temporary::save(bool replace) {

		if(filename.empty()) {
			throw system_error(EINVAL,system_category(),"No target filename");
		}

		save(filename.c_str(),replace);

	}

	File::Temporary & File::Temporary::write(const void *contents, size_t length) {

		if(fd < 0) {
			throw runtime_error("Temporary file is no longer available");
		}

		while(length) {

			ssize_t bytes = ::write(fd, contents, length);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to temporary file");
			}

			length -= bytes;
			contents = (void *) (((uint8_t *) contents) + bytes);

		}
		return *this;

	}


 }
