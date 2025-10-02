/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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

 #if defined(DEBUG) and ! defined(STATIC_LIBRARY) 

 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/url.h>
 #include <string>
 #include <udjat/net/interface.h>
 #include <udjat/tools/configuration.h>

 #ifdef HAVE_UNISTD_H
 #include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef HAVE_OPENSSL
 #include <udjat/tools/crypto.h>
 #endif // HAVE_OPENSSL

 using namespace Udjat;
 using namespace std;

 #ifdef HAVE_SMBIOS
 static int smbios_test() {
	try {
		string smbios = URL{"dmi:///BIOS"}.get();
		Logger::String{"SMBIOS information: ",smbios.c_str()}.info();
	} catch(const std::exception &e) {
		Logger::String{"Error getting SMBIOS information: ",e.what()}.error();
	}	
	return 0;
 }
 #endif // HAVE_SMBIOS

 #ifdef HAVE_OPENSSL
 static int ssl_test() {

	static const char * backends[] = {
		"legacy",
#if defined(HAVE_OPENSSL_ENGINE)
		"engine",
#endif
#if defined(HAVE_OPENSSL_PROVIDER)
		"provider",
#endif
	};

	for(const auto &backend : backends) {

		Logger::String{"-----[ Testing backend '",backend,"' ]------------------------------------------------------"}.info();
		try {

			String filename{"/tmp/test-",backend,".key"};

			Udjat::Crypto::Key pkey;

			// Test key generation
			pkey.generate(filename.c_str(),"password",2048,backend);

			string pkeystr = pkey.to_string();
			bool tss = strstr(pkeystr.c_str(),"BEGIN TSS") != nullptr;

			Logger::String{"Generated private key for ",backend," (",(tss ? "tss" : "legacy"),"):\n",pkeystr.c_str()}.info();
			pkey.save_public(String{"/tmp/test-",backend,".pub"}.c_str());

			// Test key loading
			String loaded = Udjat::Crypto::Key{}.load(filename.c_str(),"password",backend).to_string();

			Logger::String{"Reloaded private key for ",backend," (",(tss ? "tss" : "legacy"),"):\n",loaded.c_str()}.info();

			if(strcmp(loaded.c_str(),pkeystr.c_str()) != 0) {
				throw logic_error("Reloaded key does not match generated key.");
			}

		} catch(const std::exception &e) {
			Logger::String{"Error testing backend '",backend,"': ",e.what()}.error();
		}

	}


	/*
	unlink("/tmp/test-legacy.key");
	unlink("/tmp/test-legacy.pub");
	unlink("/tmp/test-engine.key");
	unlink("/tmp/test-engine.pub");
	unlink("/tmp/test-provider.key");	
	unlink("/tmp/test-provider.pub");
	unlink("/tmp/test-mixed.key");	
	unlink("/tmp/test-mixed.pub");

	debug("---[ Legacy key test ]------------------------------------------------------------");
	pkey.generate("/tmp/test-legacy.key","password",2048,"legacy");
	Logger::String{"Legacy private key:\n",pkey.to_string().c_str()}.info();
	pkey.save_public("/tmp/test-legacy.pub");
	pkey.load("/tmp/test-legacy.key","password");
	Logger::String{"Legacy private key reloaded:\n",pkey.to_string().c_str()}.info();

#if defined(HAVE_OPENSSL_ENGINE) && defined(HAVE_TPM2_TSS_ENGINE_H) && !defined(_WIN32)
	debug("---[ Engine based TPM test ]------------------------------------------------------");
	pkey.generate("/tmp/test-engine.key","password",2048,"engine");
	Logger::String{"Engine private key:\n",pkey.to_string().c_str()}.info();
	pkey.save_public("/tmp/test-engine.pub");
	pkey.load("/tmp/test-engine.key","password");
	Logger::String{"Engine private key reloaded:\n",pkey.to_string().c_str()}.info();

	if(strstr("TSS2 PRIVATE KEY",pkey.to_string().c_str()) == nullptr) {
		throw logic_error("Engine key does not look like a TPM key.");
	}

#endif // HAVE_OPENSSL_ENGINE

#if defined(HAVE_OPENSSL_PROVIDER) && !defined(_WIN32)
	if(access(STRINGIZE_VALUE_OF(LIBDIR) "/ossl-modules/tpm2.so", R_OK) != 0) {
		Logger::String{"TPM2 provider not found, skipping provider test."}.warning();
	} else {
		debug("---[ Provider based TPM test ]----------------------------------------------------");
		pkey.generate("/tmp/test-provider.key","password",2048,"provider");
		Logger::String{"Provider private key:\n",pkey.to_string().c_str()}.info();
		pkey.save_public("/tmp/test-provider.pub");
		pkey.load("/tmp/test-provider.key","password");
		Logger::String{"Provider private key reloaded:\n",pkey.to_string().c_str()}.info();

		if(strstr("TSS2 PRIVATE KEY",pkey.to_string().c_str()) == nullptr) {
			throw logic_error("Provider key does not look like a TPM key.");
		}

	}
#endif // HAVE_OPENSSL_PROVIDER
	*/

return 0;
 }
 #endif // HAVE_OPENSSL

 static int network_test() {

#ifndef _WIN32
	auto nic = Udjat::Network::Interface::Default();

	auto name = nic->name();
	if(name && *name) {
		Logger::String{"Default network interface address: ",name}.info();
	} else {
		throw logic_error("No default network interface found.");
	}	

	auto addr = nic->address().to_string();
	if(addr.empty()) {
		throw logic_error("No default network interface found.");
	} else {
		Logger::String{"Default network interface address: ",addr.c_str()}.info();
	}	

	auto mask = nic->netmask().to_string();
	if(mask.empty()) {
		throw logic_error("No default network interface netmask found.");
	} else {
		Logger::String{"Default network interface netmask: ",mask.c_str()}.info();
	}
 #endif // !_WIN32
 
	return 0;
 }

 static int config_test() {

	// This test requires a configuration file with this entries:
	//
	// [enum_test]
	//   value1 = 1
	//   value2 = 2
	//   value3 = 3
	//   value4 = 4
	auto rc = Config::for_each("enum_test",[](const char *key, const char *value) -> bool {
		Logger::String{"Configuration key: ",key," = ",value}.info();
		return false; // Continue iterating
	});

	if(!rc) {
		Logger::String{"Configuration test passed."}.info();
	} else {
		throw runtime_error{"Configuration test failed."};
	}
	return 0;

 }

 static int url_test() {

	if(URL{"/tmp/xx"}.remote()) {
		throw logic_error{"URL test failed: /tmp/xx should be local. (A)"};
	}

	if(!URL{"/tmp/xx"}.local()) {
		throw logic_error{"URL test failed: /tmp/xx should be local. (B)"};
	}

	if(URL{"http://example.com"}.local()) {
		throw logic_error{"URL test failed: http://example.com should be remote. (A)"};
	}

	if(!URL{"http://example.com"}.remote()) {
		throw logic_error{"URL test failed: http://example.com should be remote. (B)"};
	}

	Logger::String{"Scheme for empty url is '",URL{}.scheme().c_str(),"'"}.info();

	return 0;
 }

 UDJAT_API int run_udjat_unit_test(const char *name) {

	static const struct {
		const char *name;
		int (*test)();
	} tests[] = {
		{"url",url_test},
 #ifdef HAVE_OPENSSL
		{"ssl", ssl_test},
 #endif // HAVE_OPENSSL
 #ifdef HAVE_SMBIOS
		{"smbios", smbios_test},
 #endif // HAVE_SMBIOS
		{"network", network_test},
		{"config", config_test},
	};

	if(!name) {
		for(const auto &test : tests) {
			Logger::String{"Running unit test: ",test.name}.info();
			test.test();
		}
	} else {
		for(const auto &test : tests) {
			if(strcasecmp(test.name, name) == 0) {
				Logger::String{"Running unit test: ",test.name}.info();
				return test.test();
			}
		}
	}

	return 0;
 }

 #endif // DEBUG

 