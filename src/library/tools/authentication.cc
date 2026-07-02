/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/authentication.h>
 #include <udjat/tools/memory.h>
 #include <stdexcept>
 #include <udjat/tools/base64.h>

#ifdef HAVE_OPENSSL
	#include <openssl/evp.h>
	#include <openssl/rand.h>
	#include <openssl/err.h>
	#include <udjat/tools/crypto.h>
	#include <openssl/bio.h>
	#include <openssl/evp.h>
	#include <openssl/buffer.h>
#endif // HAVE_OPENSSL

 using namespace std;

 namespace Udjat {

#ifdef HAVE_OPENSSL
	constexpr size_t KEY_SIZE = 32; // 256-bit key
	constexpr size_t IV_SIZE = 12;  // 96-bit IV (Standard for GCM)
	constexpr size_t TAG_SIZE = 16; // 128-bit authentication tag

	class UDJAT_PRIVATE Controller {
	private:

		unsigned char key[KEY_SIZE];

		Controller() {
			reset();
		}

		void generate_random_bytes(unsigned char key[KEY_SIZE]) {
			if(RAND_bytes(key,KEY_SIZE) != 1) {
				throw Crypto::Exception("Failed to generate secure random bytes");
			}
		};

	public:

		~Controller() {

		}

		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}

		void reset() {
			generate_random_bytes(key);
		}

		/// @brief Encrypt payload using AES-256-GCM
		/// @param payload The payload to encrypt.
		std::string encrypt(const std::string &payload) {
			unsigned char iv[KEY_SIZE];
			generate_random_bytes(iv);

			unsigned char ciphertext[payload.size() << 1];
			unsigned char tag[TAG_SIZE];

			auto ctx = make_handle(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
			if (!ctx) {
        		throw Crypto::Exception("Failed to create EVP_CIPHER_CTX");
    		}

			// Initialize encryption operation
			if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        		throw Crypto::Exception("Failed to initialize encryption");
    		}			

			// Set IV length (default is 12, but best practice is to explicitly declare it)
    		if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_AEAD_SET_IVLEN, IV_SIZE, nullptr) != 1) {
				throw Crypto::Exception("Failed to set IV length");
			}

			// Provide the key and IV to the cipher context
    		if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key, iv) != 1) {
        		throw Crypto::Exception("Failed to set Key and IV");
    		}
			
			// Encrypt the plaintext payload
    		int len = 0;
    		if (EVP_EncryptUpdate(ctx.get(), ciphertext, &len, (unsigned char *) payload.c_str(), payload.size()) != 1) {
        		throw Crypto::Exception("Failed to encrypt plaintext");
			}
			int ciphertext_len = len;

 			// Finalize encryption (in GCM mode, this writes no extra bytes but finishes computations)
    		if (EVP_EncryptFinal_ex(ctx.get(), ciphertext + len, &len) != 1) {
        		throw Crypto::Exception("Failed to finalize encryption");
			}
			ciphertext_len += len;

			// Retrieve the authentication tag (verifies ciphertext integrity)
    		if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_AEAD_GET_TAG, TAG_SIZE, tag) != 1) {
        		throw Crypto::Exception("Failed to retrieve authentication tag");
    		}

			// Build output buffer.
			{
				size_t buflen = ciphertext_len+TAG_SIZE+IV_SIZE+2;

				unsigned char buffer[buflen];

				debug("buflen=",buflen);

				// first 2 bytes have the ciphertext len.
				*((uint16_t *) buffer) = (uint16_t) ciphertext_len;
				memcpy(
					buffer+2,
					ciphertext,
					ciphertext_len
				);
				
				memcpy(
					buffer+2+ciphertext_len,
					tag,
					TAG_SIZE
				);
				
				memcpy(
					buffer+2+ciphertext_len+TAG_SIZE,
					iv,
					IV_SIZE
				);

				return Base64::encode(buffer,buflen);
			}

		}			

		std::string decrypt(const std::string &b64) {

			size_t maxlen = b64.size()*2;
			unsigned char decoded[maxlen];
			ssize_t decoded_len;
			int ciphertext_len = 0;

			unsigned char *ciphertext;
			unsigned char *tag;
			unsigned char *iv;

			decoded_len = Base64::decode((unsigned char *) b64.c_str(),decoded,maxlen);
			if(decoded_len < 0) {
				throw runtime_error("Failed to decode base64");
			}

			// Check prefix, split buffer.
			{
				ciphertext_len = *((uint16_t *) decoded);
				ssize_t buflen = ciphertext_len+TAG_SIZE+IV_SIZE+2;

				debug("decoded_len=",decoded_len);
				debug("Required len:",buflen);
				debug("ciphertext_len=",ciphertext_len);

				if( decoded_len != buflen) {
					throw runtime_error("Failed to validate base64");
				}

				ciphertext = decoded+2;
				tag = ciphertext + ciphertext_len;
				iv = tag + TAG_SIZE;

			}

			auto ctx = make_handle(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
			if (!ctx) {
        		throw Crypto::Exception("Failed to create EVP_CIPHER_CTX");
    		}

			// Initialize decryption operation
    		if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
        		throw Crypto::Exception("Failed to initialize decryption");
    		}

			// Set IV length
			if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_AEAD_SET_IVLEN, IV_SIZE, nullptr) != 1) {
				throw Crypto::Exception("Failed to set IV length");
			}

			// Provide the key and IV
    		if (EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, key, iv) != 1) {
        		throw Crypto::Exception("Failed to set Key and IV");
    		}

			// Provide the expected verification tag
    		if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_AEAD_SET_TAG, TAG_SIZE, tag) != 1) {
        		throw Crypto::Exception("Failed to set verification tag");
    		}			

			unsigned char plaintext[ciphertext_len*2];

			// Decrypt the ciphertext
			int len = 0;
			if (EVP_DecryptUpdate(ctx.get(), plaintext, &len, ciphertext, ciphertext_len) != 1) {
				throw Crypto::Exception("Failed to decrypt ciphertext");
			}
			int plaintext_len = len;	
			
			// Finalize decryption (This performs tag verification)
			// If the ciphertext has been modified or the tag is invalid, this function returns <= 0.
			int ret = EVP_DecryptFinal_ex(ctx.get(), plaintext + len, &len);
			if (ret <= 0) {
				throw std::runtime_error("Authentication failed! Token is invalid or has been modified.");
			}		

			plaintext_len += len;

			return std::string{(const char *) plaintext,(size_t) plaintext_len};

		}
	
	};
#endif // HAVE_OPENSSL

	Authentication::Authentication(Level level) : current_level{level} {
	}

	Authentication::Authentication(const char *u, Level level) : current_level{level}, username{u} {

	}

	void Authentication::reset() {
#ifdef HAVE_OPENSSL
		Controller::getInstance().reset();
#else
		throw runtime_error("Authentication engine is not available");
#endif // HAVE_OPENSSL
	}

	Authentication::~Authentication() {

	}

	std::string Authentication::encrypt(const std::string &token) {
#ifdef HAVE_OPENSSL
		return Controller::getInstance().encrypt(token);
#else
		throw runtime_error("Authentication engine is not available");
#endif // HAVE_OPENSSL
	}

	std::string Authentication::decrypt(const std::string &b64) {
#ifdef HAVE_OPENSSL
		return Controller::getInstance().decrypt(b64);
#else
		throw runtime_error("Authentication engine is not available");
#endif // HAVE_OPENSSL
	}


 }

