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

 #define LOG_DOMAIN "sslkey"

 #include <config.h>
 #include <private/crypto.h>
 #include <udjat/tools/file/text.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file/temporary.h>
 #include <cstdio>

 #ifdef HAVE_UNISTD_H
 #include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <udjat/tools/crypto.h>
 #include <memory>

 using namespace std;


 namespace Udjat {

	Crypto::Key::operator bool() const noexcept {
		if(backend && *backend) {
			return true;
		}
		return false;
	}

	Crypto::Key::operator EVP_PKEY *() const {
		if(backend) {
			return *backend;
		}
		throw runtime_error("Private key is not loaded");
	}

	Crypto::Key::Key(const char *filename, const char *password, bool autogenerate) {

		if(filename && *filename) {

			this->filename = filename;

			if(access(filename,R_OK) == 0) {
				// File exists, load it.
				load(filename,password);	
				return;
			}

			debug("File ",filename," does not exist or is not readable");
		}

		if(autogenerate) {
			Logger::String{"Generating new private key in ",filename}.trace();
			generate(filename, password, (size_t) Config::Value<unsigned int>{"crypto","mbits",2048}.get());
			return;
		}

		throw system_error(EINVAL,system_category(),"filename is required to load private key");

	}

	Crypto::Key::~Key() {
	}

	void Crypto::Key::generate(const char *filename, const char *passwd, size_t mbits, const char *defmode) {

		if(filename && *filename) {
			this->filename = filename;
		} else {
			this->filename.clear();
		}

		String mode;
		if(defmode && *defmode) {
			mode = defmode;
		} else {
			mode = Config::Value<string>{"crypto","genkey","auto"}.c_str();
		}

		backend = BackEnd::Factory(mode);

		if(!backend->subproc(filename, passwd, mbits)) {
			// No subprocess defined for this backend, use it directly.
			backend->generate(filename, passwd, mbits);
		}	

	}

	void Crypto::Key::load(const char *filename, const char *passwd) {

		throw runtime_error("TPM2 key loading not implemented yet");
		
		/*

        static const char *tpm2_tags[] = {
                "-----BEGIN TSS2 PRIVATE KEY-----",
                "-----BEGIN TSS2 KEY BLOB-----",
        };


		File::Text text{filename};
		String mode{"legacy"};

		for(const char *tpm_tag : tpm2_tags) {
			if(strstr(text.c_str(),tpm_tag)) {
				Logger::String{"Found TPM2 key on ",filename}.trace();
#if defined(HAVE_OPENSSL_PROVIDER)
				mode = Config::Value<string>{"crypto","tpm-engine","provider"}.c_str();
#elif defined(HAVE_OPENSSL_ENGINE)
				mode = Config::Value<string>{"crypto","tpm-engine","engine"}.c_str();
#else
				throw runtime_error("No OpenSSL backend available for TPM2 key");
#endif
				break;
			}
		}

		backend = BackEnd::Factory(mode);
		backend->load(filename,passwd);
		*/

	}

	void Crypto::Key::save_private(const char *filename, const char *password) {
		backend->save_private(filename, password);
	}

	void Crypto::Key::save_public(const char *filename) {
		backend->save_public(filename);
	}

	std::string Crypto::Key::to_string() const {
		if(backend) {
			return backend->get_private();
		}
		return "";
	}


 }

