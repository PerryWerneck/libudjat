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

#include "private.h"
#include <udjat/url.h>

#ifdef HAVE_WINHTTP

	#define INTERNET_TEXT wchar_t * __attribute__((cleanup(wchar_t_cleanup)))
	#define INTERNET_HANDLE HINTERNET __attribute__((cleanup(hinternet_t_cleanup)))

	static void wchar_t_cleanup(wchar_t **buf) {
		if(*buf) {
			free(*buf);
			*buf = NULL;
		}
	}

	static void hinternet_t_cleanup(HINTERNET *handle) {
		if(*handle) {
			WinHttpCloseHandle(*handle);
			*handle = 0;
		}
	}

	Udjat::HTTP::Client::Client(const char *u) : url(u) {
	}

	Udjat::HTTP::Client::~Client() {
	}

	Udjat::HTTP::Client::Worker * Udjat::HTTP::Client::Worker::getInstance(HTTP::Client *client) {
		return new Worker(client);
	}

	std::string Udjat::HTTP::Client::get() {

		string response;
		Worker * worker = Worker::getInstance(this);

		try {

			response = worker->call("GET");

		} catch(...) {

			delete worker;
			throw;

		}

		delete worker;
		return response;

	}

	std::string Udjat::HTTP::Client::post(const char *payload) {

		string response;
		Worker * worker = Worker::getInstance(this);

		try {

			response = worker->call("POST",payload);

		} catch(...) {

			delete worker;
			throw;

		}

		delete worker;
		return response;

	}

	void Udjat::HTTP::Client::set(const HTTP::Client::Header &header) {
		headers.push_back(header);
		throw system_error(ENOTSUP,system_category(),"Not implemented");
	}

	Udjat::HTTP::Client::Worker::Worker(HTTP::Client *c) : client(c) {

		if(client->url.empty()) {
			throw runtime_error("Empty URL");
		}

		// Open HTTP session
		// https://docs.microsoft.com/en-us/windows/desktop/api/winhttp/nf-winhttp-winhttpopenrequest
		static const char * userAgent = PACKAGE_NAME "/" PACKAGE_VERSION;
		wchar_t wUserAgent[256];
		mbstowcs(wUserAgent, userAgent, strlen(userAgent)+1);

		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopen
		this->session =
			WinHttpOpen(
				wUserAgent,
#ifdef WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
				WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
#else
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
#endif //
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				0
			);

		if(!this->session) {
			throw Win32::Exception(this->client->url + ": Can't open HTTP session");
		}

		// Set timeouts.
		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsettimeouts
		Config::Value<int> timeout("http","Timeout",6000);

		if(!WinHttpSetTimeouts(
				this->session,
				Config::Value<int>("http","ResolveTimeout",timeout.get()).get(),
				Config::Value<int>("http","ConnectTimeout",timeout.get()).get(),
				Config::Value<int>("http","SendTimeout",timeout.get()).get(),
				Config::Value<int>("http","ReceiveTimeout",timeout.get()).get()
			)) {

			throw Win32::Exception(this->client->url + ": Can't set HTTP session timeouts");
		}

		/*
		{
			size_t hs = client->url.find("://");
			if(hs == string::npos) {
				throw runtime_error(string{"Can't parse hostname from '"} + client->url + "'");
			}
			hs += 3;

			size_t he = client->url.find('/',hs);

			if(he == string::npos) {
				hostname.assign(client->url.c_str()+hs);
				path.clear();
			} else {
				hostname = string(client->url.c_str()+hs,(he-hs));
				path.assign(client->url.c_str()+he+1);
			}
		}

		this->secure = (strncasecmp(this->client->url.c_str(),"https://",8) == 0);
		*/

	}

	Udjat::HTTP::Client::Worker::~Worker() {
		WinHttpCloseHandle(this->session);
	}

	static void CrackUrl(wchar_t *pwszUrl, URL_COMPONENTS &urlComp) {

		ZeroMemory(&urlComp, sizeof(urlComp));
		urlComp.dwStructSize 		= sizeof(urlComp);
		urlComp.dwSchemeLength    	= (DWORD)-1;
		urlComp.dwHostNameLength  	= (DWORD)-1;
		urlComp.dwUrlPathLength   	= (DWORD)-1;
		urlComp.dwExtraInfoLength 	= (DWORD)-1;

		if (!WinHttpCrackUrl(pwszUrl, (DWORD)wcslen(pwszUrl), 0, &urlComp)) {
			throw Udjat::Win32::Exception("Invalid URL on HTTP request");
		}
	}

	HINTERNET Udjat::HTTP::Client::Worker::connect() {

		URL_COMPONENTS urlComp;

		INTERNET_TEXT pwszUrl = (wchar_t *) malloc(client->url.size()*3);
		mbstowcs(pwszUrl, client->url.c_str(), client->url.size()+1);

		CrackUrl(pwszUrl, urlComp);

		HINTERNET connection =
			WinHttpConnect(
				this->session,
				urlComp.lpszHostName,
				urlComp.nPort,
				0
			);

		if(!connection) {
			throw Win32::Exception(this->client->url + ": Can't connect to host");
		}

		return connection;

	}

	std::string Udjat::HTTP::Client::Worker::wait(HINTERNET request) {

		// Wait for response
		if(!WinHttpReceiveResponse(request, NULL)) {
			throw Win32::Exception(this->client->url + ": Error receiving response");
		}

		// Get status code.
		{
			DWORD dwStatusCode = 0;
			DWORD dwSize = sizeof(DWORD);

			if(!WinHttpQueryHeaders(
					request,
					WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
					WINHTTP_HEADER_NAME_BY_INDEX,
					&dwStatusCode,
					&dwSize,
					WINHTTP_NO_HEADER_INDEX
				)) {
					throw Udjat::Win32::Exception("Cant get HTTP status code");
				}

			if(dwStatusCode != 200) {

				wchar_t buffer[1024];
				dwSize = 1023;

				ZeroMemory(buffer, sizeof(buffer));

				WinHttpQueryHeaders(
					request,
					WINHTTP_QUERY_STATUS_TEXT,
					WINHTTP_HEADER_NAME_BY_INDEX,
                    buffer,
                    &dwSize,
					WINHTTP_NO_HEADER_INDEX
				);

				char text[1024];
				ZeroMemory(text, sizeof(text));
				wcstombs(text, buffer, 1023);

				throw Udjat::HTTP::Exception(dwStatusCode,text);
			}

		}

		DWORD szResponse = 0;
		if(!WinHttpQueryDataAvailable(request, &szResponse)) {
			return "";
		}

		char * text = new char[szResponse+1];
		memset(text,0,szResponse+1);

		if(!WinHttpReadData(request,text,szResponse,&szResponse)){

			delete[] text;

			throw Win32::Exception(this->client->url + ": Can't read data");
		}

		string response(text);

		delete[] text;

		return response;

	}

	HINTERNET Udjat::HTTP::Client::Worker::open(HINTERNET connection, const LPCWSTR pwszVerb) {

		URL_COMPONENTS urlComp;

		INTERNET_TEXT pwszUrl = (wchar_t *) malloc(client->url.size()*3);
		mbstowcs(pwszUrl, client->url.c_str(), client->url.size()+1);

		CrackUrl(pwszUrl, urlComp);

		HINTERNET req =
			WinHttpOpenRequest(
				connection,
				pwszVerb,
				urlComp.lpszUrlPath,
				NULL,
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				(urlComp.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)
			);

		if(!req) {
			throw Win32::Exception(this->client->url + ": Can't create request");
		}

		WinHttpSetOption(req, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, WINHTTP_NO_CLIENT_CERT_CONTEXT, 0);

		return req;

	}

	void Udjat::HTTP::Client::Worker::send(HINTERNET request, const char *payload) {

		INTERNET_TEXT	lpszHeaders = NULL;
		DWORD			dwHeadersLength = 0;

		if(!client->headers.empty()) {
			throw runtime_error("There's no support for custom headers (yet)");
		}

		size_t sz = 0;
		if(payload) {
			sz = strlen(payload);
		}

		if(!WinHttpSendRequest(
				request,
				(lpszHeaders ? lpszHeaders : WINHTTP_NO_ADDITIONAL_HEADERS), dwHeadersLength,
				(LPVOID) (payload ? payload : WINHTTP_NO_REQUEST_DATA),
				sz,
				sz,
				0)
			) {
			throw Win32::Exception(this->client->url + ": Can't send request");
		}

	}

	std::string Udjat::HTTP::Client::Worker::call(const char *verb, const char *payload) {

		INTERNET_TEXT wVerb = (wchar_t *) malloc(strlen(verb)*3);
		mbstowcs(wVerb, verb, strlen(verb)+1);

		INTERNET_HANDLE	connection = connect();
		INTERNET_HANDLE	request = open(connection,wVerb);

		if(payload) {

			if(Config::Value<bool>("http","trace-payload",true).get()) {
				cout << "http\tPosting to " << client->url << endl << payload << endl;
			}
			send(request, payload);

		} else {

			send(request);

		}

		return wait(request);
	}

	/*
	std::string Udjat::HTTP::Client::Worker::get() {

		INTERNET_HANDLE	connection = connect();
		INTERNET_HANDLE	request = open(connection,L"GET");

		send(request);
		return wait(request);

	}

	std::string Udjat::HTTP::Client::Worker::post() {

		INTERNET_HANDLE	connection = connect();
		INTERNET_HANDLE	request = open(connection,L"POST");

		string payload;
		this->client->getPostPayload(payload);

		if(Config::Value<bool>("http","trace-payload",true).get()) {
			cout << "http\tPosting to " << client->url << endl << payload << endl;
		}

		send(request, payload.c_str());
		return wait(request);

	}
	*/

#endif // HAVE_WINHTTP
