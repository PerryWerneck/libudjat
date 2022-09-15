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

 #include <config.h>
 #include <iconv.h>
 #include <udjat/win32/string.h>
 #include <stdexcept>
 #include <cstring>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	Win32::String::String() {
		local = iconv_open("CP1252","UTF-8");
	}

	Win32::String::String(const char *winstr) : String() {
		assign(winstr);
	}

	Win32::String::~String() {
		iconv_close(local);
	}

	Win32::String & Win32::String::assign(const char *winstr) {

		iconv(local,NULL,NULL,NULL,NULL);	// Reset state

		size_t	  	  szIn		= strlen(winstr);
		size_t	  	  szOut		= szIn*2;

#if defined(WINICONV_CONST)
		WINICONV_CONST char 	* inBuf 	= (WINICONV_CONST char *) winstr;
#elif defined(ICONV_CONST)
		ICONV_CONST char		* inBuf 	= (ICONV_CONST char *) winstr;
#else
		char 		 			* inBuf 	= (char *) winstr;
#endif // WINICONV_CONST

		char * outBuff	= new char[szOut+1];
		char * ptr;
		memset(ptr = outBuff,0,szOut+1);

		if(iconv(local,&inBuf,&szIn,&ptr,&szOut) == ((size_t) -1)) {
			cerr << "win32\tError '" << strerror(errno) << "' converting '" << inBuf << "' to UTF-8" << endl;
			strcpy(outBuff,inBuf);
			for(char * ptr = outBuff; *ptr; ptr++) {
				if(*ptr < ' ') {
					*ptr = '?';
				}
			}

#ifdef DEBUG
			cout << "---> " << winstr << " <---" << endl << outBuff << endl;
#endif // DEBUG

		} else {
			outBuff[szOut] = 0;
		}

		std::string::assign(outBuff);

		delete[] outBuff;
		return *this;
	}

 }
