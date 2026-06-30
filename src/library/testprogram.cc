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

 #if defined(DEBUG) and ! defined(LIBUDJAT_STATIC) 

 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <private/logger.h>
 #include <udjat/module.h>
 #include <udjat/tools/url.h>
 #include <string>
 #include <udjat/net/interface.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/unit-test.h>
 #include <udjat/tools/memory.h>
 #include <udjat/ui/console/progress.h>
 #include <udjat/ui/console.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/mainloop.h>

 #ifdef HAVE_UNISTD_H
 #include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifndef _WIN32
  #include <udjat/tools/system.h>
 #endif // !_WIN32

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
 	static void test_ssl(const char *backend) {

		String filename{"/tmp/test-",backend,".key"};

		Udjat::Crypto::Key pkey;

		// Test key generation
		pkey.generate(filename.c_str(),"password",2048,backend);

		string pkeystr = pkey.to_string();
		bool tss = strstr(pkeystr.c_str(),"BEGIN TSS") != nullptr;

		Logger::String{"Generated private key for ",backend," (",(tss ? "tss" : "legacy"),"):\n",pkeystr.c_str()}.info();
		pkey.save_public(String{"/tmp/test-",backend,".pub"}.c_str());

		// Test encription/decription
		{
			size_t encripted_len = 0;
			size_t decripted_len = 0;

			const char *buffer = "Simple string to test crypto functions";

			auto encripted = make_handle<void>(pkey.encrypt(buffer,encripted_len),free);

			Logger::String{"The encripted block has ",encripted_len," bytes"}.info();

			auto decrypted = make_handle<void>(pkey.decrypt(encripted.get(),encripted_len,decripted_len),free);

			debug("Decrypted string: '",((char *) decrypted.get()),"'");

			if(strcmp(buffer,(const char *) decrypted.get())) {
				throw runtime_error("Error decripting data block");
			} else {
				Logger::String{"Decripted block is ok"}.info();
			}

		}

		// Test sign/verify
		{
			size_t siglen;
			unsigned int diglen;

			const char *buffer = "Simple string to test crypto functions";
			void *digest = pkey.digest(buffer,diglen);

			Logger::String{"The digest block has ",diglen," bytes"}.info();

			void *sig = pkey.sign(digest,diglen,siglen);

			Logger::String{"The signed block has ",siglen," bytes"}.info();

			if(pkey.verify(sig,siglen,digest,diglen)) {
				Logger::String{"Signed block is ok"}.info();
			} else {
				free(digest);
				free(sig);
				throw runtime_error("Error sigining data block");
			}

			free(digest);
			free(sig);
		}

		// Test key loading
		Logger::String{"Reloading private key for ",backend," from file."}.info();
		String loaded = Udjat::Crypto::Key{}.load(filename.c_str(),"password",backend).to_string();

		Logger::String{"Reloaded private key for ",backend," (",(tss ? "tss" : "legacy"),"):\n",loaded.c_str()}.info();

		if(strcmp(loaded.c_str(),pkeystr.c_str()) != 0) {
			throw logic_error("Reloaded key does not match generated key.");
		}

		Logger::String{"-----[ Finished test of backend '",backend,"' ]---------------------------------------------"}.notice();

	}
 #endif // HAVE_OPENSSL


//  #ifdef HAVE_OPENSSL
//  static int ssl_test() {

// 	static const char * backends[] = {
// 		"legacy",
// #if defined(HAVE_OPENSSL_ENGINE)
// 		"engine",
// #endif
// #if defined(HAVE_OPENSSL_PROVIDER)
// 		"provider",
// #endif
// 	};

// 	for(const auto &backend : backends) {

// 		Logger::String{"-----[ Testing backend '",backend,"' ]------------------------------------------------------"}.notice();
// 		try {

// 			String filename{"/tmp/test-",backend,".key"};

// 			Udjat::Crypto::Key pkey;

// 			// Test key generation
// 			pkey.generate(filename.c_str(),"password",2048,backend);

// 			string pkeystr = pkey.to_string();
// 			bool tss = strstr(pkeystr.c_str(),"BEGIN TSS") != nullptr;

// 			Logger::String{"Generated private key for ",backend," (",(tss ? "tss" : "legacy"),"):\n",pkeystr.c_str()}.info();
// 			pkey.save_public(String{"/tmp/test-",backend,".pub"}.c_str());

// 			// Test encription/decription
// 			{
// 				size_t encripted_len = 0;
// 				size_t decripted_len = 0;

// 				const char *buffer = "Simple string to test crypto functions";

// 				auto encripted = make_handle<void>(pkey.encrypt(buffer,encripted_len),free);

// 				Logger::String{"The encripted block has ",encripted_len," bytes"}.info();

// 				auto decrypted = make_handle<void>(pkey.decrypt(encripted.get(),encripted_len,decripted_len),free);

// 				debug("Decrypted string: '",((char *) decrypted.get()),"'");

// 				if(strcmp(buffer,(const char *) decrypted.get())) {
// 					throw runtime_error("Error decripting data block");
// 				} else {
// 					Logger::String{"Decripted block is ok"}.info();
// 				}

// 			}

// 			// Test sign/verify
// 			{
// 				size_t siglen;
// 				unsigned int diglen;

// 				const char *buffer = "Simple string to test crypto functions";
// 				void *digest = pkey.digest(buffer,diglen);

// 				Logger::String{"The digest block has ",diglen," bytes"}.info();

// 				void *sig = pkey.sign(digest,diglen,siglen);

// 				Logger::String{"The signed block has ",siglen," bytes"}.info();

// 				if(pkey.verify(sig,siglen,digest,diglen)) {
// 					Logger::String{"Signed block is ok"}.info();
// 				} else {
// 					free(digest);
// 					free(sig);
// 					throw runtime_error("Error sigining data block");
// 				}

// 				free(digest);
// 				free(sig);
// 			}

// 			// Test key loading
// 			Logger::String{"Reloading private key for ",backend," from file."}.info();
// 			String loaded = Udjat::Crypto::Key{}.load(filename.c_str(),"password",backend).to_string();

// 			Logger::String{"Reloaded private key for ",backend," (",(tss ? "tss" : "legacy"),"):\n",loaded.c_str()}.info();

// 			if(strcmp(loaded.c_str(),pkeystr.c_str()) != 0) {
// 				throw logic_error("Reloaded key does not match generated key.");
// 			}

// 		} catch(const std::exception &e) {
// 			Logger::String{"Error testing backend '",backend,"': ",e.what()}.error();
// 		}

// 		Logger::String{"-----[ Finished test of backend '",backend,"' ]---------------------------------------------"}.notice();
// 	}


// 	return 0;
//  }
//  #endif // HAVE_OPENSSL

 static int network_test() {

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

 static int string_test() {

	static const char *xml = {
		"<root>"
		"<template name='isolinux.cfg' url='file://${template-dir}/isolinux.cfg' escape-control-characters='no' />"
		"</root>"
	};

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(xml);
	if(!result) {
		throw runtime_error{String{"Error parsing XML string: ",result.description()}};
	}
	auto root = XML::Node{doc}.child("root");
	auto template_node = root.child("template");

	String str{template_node, "url", true};
	Logger::String{"Extracted string from XML: '",str.c_str(),"'."}.info();
	if(strcmp(str.c_str(),"file://${template-dir}/isolinux.cfg") != 0) {
		throw logic_error{"String test failed: extracted string does not match expected value."};
	}	

	return 0;
 }

 static int tmpfile_test() {

	{
		String filename = File::Temporary::create(100);
		Logger::String{"Temporary file '",filename,"' created"}.info();
	}

	{
		int fd = File::Temporary::open(100);
		Logger::String{"Temporary file '",fd,"' open"}.info();
		::close(fd);
	}

	return 0;
 }

#if defined(HAVE_IBMTSS) && defined(HAVE_OPENSSL)
 	static int tpm_test() {
		TPM::probe();
		return 0;
	}
#endif 		

#ifndef _WIN32
 static int sysconfig_test() {

	static const struct {
		const char *source;
		const char *expected;
	} test_data[] = {
		{
		"\nTEST_VALUE=old-value\n",
		"\nTEST_VALUE=new\n",
		},
		{
		"\nTEST_VALUE=\"old-value\"\n",
		"\nTEST_VALUE=\"new\"\n",
		},
		{
		"\nTEST_VALUE=\'old-value\'\n",
		"\nTEST_VALUE=\'new\'\n",
		},
		{
		"\nTEST_VALUE = old-value\n",
		"\nTEST_VALUE = new\n",
		},
		{
		"\nTEST_VALUE = \"old-value\"\n",
		"\nTEST_VALUE = \"new\"\n",
		},
		{
		"\nTEST_VALUE = \'old-value\'\n",
		"\nTEST_VALUE = \'new\'\n",
		},

	};

	for(const auto &data : test_data) {
		String text{data.source};

		System::Config::File::replace(text,"TEST_VALUE","new");

		if(strcmp(text.c_str(),data.expected)) {
			Logger::String{"Result:\n",text.c_str(),"\n"}.error();
			Logger::String{"Expected:\n",data.expected,"\n"}.info();
			return EINVAL;
		}
	}

	Logger::String{"Sysconfig manipulation seens ok"}.info();
	return 0;

 }
#endif // !_WIN32

 UDJAT_API void enum_udjat_unit_tests(Udjat::UnitTests &tests) noexcept {

		debug(__FUNCTION__," begin -> ",tests.size());

#ifndef _WIN32
		tests.append(
			UnitTests::Worker{
				"Test sysconfig",
				[]() {
					sysconfig_test();
					return true;
				}
			},
#endif // !_WIN32

			UnitTests::Worker{
				"hexstring", "Test to hex string",
				[]() {
					cout << endl;
					for(size_t ix = 0; ix < 10; ix++) {
						cout << ix << "=" << to_hex_string(ix) << endl;
					}
					cout << endl;
					return true;
				}
			},

#if defined(HAVE_IBMTSS) && defined(HAVE_OPENSSL)
			UnitTests::Worker{
				"Test TPM access",
				[]() {
					tpm_test();
					return true;
				}
			},
#endif 		
			UnitTests::Worker{
				"Test tempfile handler",
				[]() {
					tmpfile_test();
					return true;
				}
			},
			UnitTests::Worker{
				"Test URL engine",
				[]() {
					url_test();
					return true;
				}
			},
#ifdef HAVE_OPENSSL
			UnitTests::Worker{
				"ssl-legacy", "Test OpenSSL Legacy backend",
				[]() {
					test_ssl("legacy");
					return true;
				}
			},
#if defined(HAVE_OPENSSL_ENGINE)
			UnitTests::Worker{
				"ssl-engine", "Test OpenSSL engine backend",
				[]() {
					test_ssl("engine");
					return true;
				}
			},
#endif
#if defined(HAVE_OPENSSL_PROVIDER)
			UnitTests::Worker{
				"ssl-provider", "Test OpenSSL provider backend",
				[]() {
					test_ssl("provider");
					return true;
				}
			},
#endif
#endif // HAVE_OPENSSL
#ifdef HAVE_SMBIOS
			UnitTests::Worker{
				"smbios","Test SMBIOS Access",
				[]() {
					smbios_test();
					return true;
				}
			},
#endif // HAVE_SMBIOS
			UnitTests::Worker{
				"conffile","Test configuration file access",
				[]() {
					config_test();
					return true;
				}
			},
			UnitTests::Worker{
				"string","Test String manipulation engine",
				[]() {
					string_test();
					return true;
				}
			},
			UnitTests::Worker{
				"netinfo", "Obtain NIC info",
				[]() {
					network_test();
					return true;
				}
			},
			UnitTests::Worker{
				"animation", "Test console animations",
				[]() {

					Console::Animation animations[] = {
						Console::Animation::Style::PlainText,
						Console::Animation::Style::Simple,
						Console::Animation::Style::Braille,
						Console::Animation::Style::Circle
					};

					cout << "\n\n" << flush;
					
					for(size_t count = 0; count < 100; count++) {
						cout << '\r';
						for(auto &animation : animations) {
							cout << animation << " ";
						}
						cout << " " << count << flush;
						usleep(500000);
					}

					cout << "\n\n" << flush;

					return true;
				}
			},
			UnitTests::Worker{
				"progress", "Test console progress bar",
				[]() {

					cout << "\n\n" << flush;
					
					{
						Console::Progress progress{"Testing progress bar"};

						progress.set(Console::BrightWhiteForeground);
						progress.set(10,10);
						sleep(5);

						progress.set(Console::GreenForeground);
						for(size_t ix = 0; ix < 400;ix++) {
							progress.set(ix/4,100,false);
							if(ix == 200) {
								progress.set(Console::YellowForeground);
							} else if(ix == 250) {
								progress.set(Console::RedForeground);
							}
							usleep(50000);
						}
						progress.set(10,10,false);
					}

					cout << "\n\n" << flush;

					return true;
				}
			},
			UnitTests::Worker{
				"timer", "Test timers",
				[]() {

					cout << "\n\n" << flush;
					
					MainLoop &mainloop = MainLoop::getInstance();

					auto timer = mainloop.TimerFactory(-1,[&](){
						cout << "Timer expired" << endl;
						mainloop.quit();
						return true;
					});

					cout << "Timer is " << (timer->enabled() ? "enabled" : "disabled") << endl;

					timer->set(10000);

					cout << "Timer is " << (timer->enabled() ? "enabled" : "disabled") << endl;

					mainloop.run();

					cout << endl << endl;

					timer->set(-1);

					cout << "Timer is " << (timer->enabled() ? "enabled" : "disabled") << endl;

					delete timer;

					return true;
				}
			},
			UnitTests::Worker{
				"Test console status message",
				[]() {

					cout << "\n\n" << flush;
					
					Console::status(Logger::Notice,"test","Notification message");
					Console::status(Logger::Info,"test","Successs message");
					Console::status(Logger::Warning,"test","Warning message");
					Console::status(Logger::Error,"test","Error message");
					Console::status(Logger::Trace,"test","Trace message");
					Console::status(Logger::Debug,"test","Debug message");

					cout << "\n\n" << flush;

					return true;
				}
			}
		);

 		debug(__FUNCTION__," end -> ",tests.size());
}

 #endif // DEBUG

 