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
 #include <udjat/win32/charset.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>
 #include <cstring>
 #include <iostream>
 #include <mutex>

 using namespace std;

 namespace Udjat {

 	mutex Win32::Charset::guard;

	Win32::Charset::Charset(const char *tocode, const char *fromcode) {
		lock_guard<mutex> lock(guard);
		icnv = iconv_open(tocode,fromcode);
	}

	Win32::Charset::~Charset() {
		lock_guard<mutex> lock(guard);
		iconv_close(icnv);
	}

	std::string Win32::Charset::convert(const char *text) {
		std::string rc;
		convert(text,rc);
		return rc;
	}

	std::string Win32::Charset::from_windows(const char *str) {
		static Win32::Charset converter{
			"UTF-8",
			Win32::Charset::system()
		};

		return converter.convert(str);
	}

	std::string Win32::Charset::to_windows(const char *str) {
		static Win32::Charset converter{
			Win32::Charset::system(),
			"UTF-8"
		};

		return converter.convert(str);
	}

	void Win32::Charset::convert(const char *from, std::string &to) {
		convert(from,to,strlen(from));
	}

	//void Win32::Charset::convert(const PWSTR from, std::string &to) {
	//	convert((const char *) from,to,wcslen(from));
	//}

	void Win32::Charset::convert(const char *from, std::string &to, size_t length) {
		lock_guard<mutex> lock(guard);

		iconv(icnv,NULL,NULL,NULL,NULL);	// Reset state

		size_t	  	szIn	= length;
		size_t		buflen	= szIn*2;
		size_t	  	szOut	= buflen;

#if defined(WINICONV_CONST)
		WINICONV_CONST char 	* inBuf 	= (WINICONV_CONST char *) from;
#elif defined(ICONV_CONST)
		ICONV_CONST char		* inBuf 	= (ICONV_CONST char *) from;
#else
		char 		 			* inBuf 	= (char *) from;
#endif // WINICONV_CONST

		char outBuff[buflen+1];
		memset(outBuff,' ',buflen+1);
		outBuff[buflen] = 0;

		char * ptr = outBuff;

		if(iconv(icnv,&inBuf,&szIn,&ptr,&szOut) == ((size_t) -1) || !szOut) {
			trace("Error ",strerror(errno));
			to.assign(from);
			return;
		}

		to.assign(outBuff,(buflen-szOut));

	}

	const char * Win32::Charset::system() {
		char buffer[10];
		snprintf(buffer,9,"CP%u",GetACP());
		return Quark(buffer).c_str();
	}

	Win32::UTF8String::UTF8String() : Win32::Charset(Win32::Charset::system(),"UTF-8") {
	}

	Win32::UTF8String::UTF8String(const char *winstr) : UTF8String() {
		assign(winstr);
	}

	Win32::UTF8String & Win32::UTF8String::assign(const char *winstr) {
		convert(winstr,*this);
		return *this;
	}

	Win32::UTF8String::~UTF8String() {
	}

	Win32::Win32String::Win32String() : Win32::Charset("UTF-8", Win32::Charset::system()) {
	}

	Win32::Win32String::Win32String(const char *utfstr) : Win32String() {
		assign(utfstr);
	}

	Win32::Win32String & Win32::Win32String::assign(const char *utfstr) {
		convert(utfstr,*this);
		return *this;
	}

	Win32::Win32String::~Win32String() {
	}

 }
