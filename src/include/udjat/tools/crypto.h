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

#pragma once

#include <udjat/defs.h>
#include <string>
#include <openssl/x509.h>
#include <udjat/tools/exception.h>
#include <memory>

namespace Udjat {

	namespace TPM {

		/// @brief Probes if a TPM exists and is available.
		/// @param except If true, throws an exception if a TPM device is found but is not functional or accessible.
		/// @param force If true, forces the TPM state checks even if the current user is not root.
		/// @retval true A TPM device is found and is functional.
		/// @retval false No TPM device is found, or it is not functional/accessible (and 'except' is false).
		UDJAT_API bool probe(const bool except = true, bool force = false);

	}

	namespace Crypto {

		class UDJAT_API Exception : public Udjat::Exception {
		public:
			Exception(const char *msg);
			
			Exception(const std::string &msg) : Exception{msg.c_str()} {
			}
		};

		class BackEnd;
		
		class UDJAT_API Key {
		private:

			std::string filename;
			std::shared_ptr<BackEnd> backend;

		protected:

			/// @brief Loads or generates a private key.
			/// @param filename The file where the private key is stored.
			/// If the file does not exist, it will be created.
			/// If the file exists, it will be loaded.	
			/// @param password The password to protect the private key.
			/// If the file does not exist, this password will be used to encrypt the private
			/// key when it is generated.
			/// If the file exists, this password will be used to decrypt the private key.
			/// If the password is empty, the private key will not be encrypted.
			/// If the file exists and the password is empty, the private key will be loaded
			/// without decryption.
			/// @param autogenerate If true, the private key will be generated if the file does not exist. 
			void initialize(const char *filename, const char *password, bool autogenerate = false);
		
		public:

			/// @brief Build an empty private key.
			Key() {
			}

			Key(const Key &) = delete;
			Key & operator=(const Key &) = delete;
			Key(Key &&) = delete;
			Key & operator=(Key &&) = delete;
							
			/// @brief Loads or generates a private key.
			/// @param filename The file where the private key is stored.
			/// If the file does not exist, it will be created.
			/// If the file exists, it will be loaded.	
			/// @param password The password to protect the private key.
			/// If the file does not exist, this password will be used to encrypt the private
			/// key when it is generated.
			/// If the file exists, this password will be used to decrypt the private key.
			/// If the password is empty, the private key will not be encrypted.
			/// If the file exists and the password is empty, the private key will be loaded
			/// without decryption.
			/// @param autogenerate If true, the private key will be generated if the file does not exist. 
			Key(const char *filename, const char *password, bool autogenerate = false);
			~Key();

			operator bool() const noexcept;

			operator EVP_PKEY *() const;

			/// @brief Generates a new private key.
			/// @param mbits The size of the key in bits. Default is 2048 bits.
			/// @param mode The mode to generate the key, legacy, engine, provider or auto.
			/// If not specified, the mode will be taken from the configuration file.
			void generate(const char *filename, const char *passwd, size_t mbits = 2048, const char *mode = nullptr);

			/// @brief Loads private key from file.
			/// @param filename The file where the private key is stored.
			/// @param password The password to protect the private key.
			/// @param defmode The backend mode to use, legacy, engine or provider.
			/// If not specified, the mode will be taken from the configuration file.
			Key & load(const char *filename, const char *passwd = nullptr, const char *defmode = nullptr);

			/// @brief Save private key to file.
			/// @param filename The file where the private key will be stored.
			/// @param password The password to protect the private key.
			void save_private(const char *filename, const char *passwd = NULL);

			/// @brief Save public key to file.
			/// @param filename The file where the public key will be stored.
			void save_public(const char *filename);

			/// @brief Get string representation of the private key.
			/// @return The private key in PEM format.
			std::string to_string() const;

			/// @brief Encrypt data.
			/// @param data The data to encrypt.
			/// @param size The size of the input data.
			/// @param outsize The size of output data.
			/// @return A pointer to the encrypted data, release it with free().
			void * encrypt(const void *data, size_t size, size_t &outsize);

			inline void * encrypt(const char *data, size_t &outsize) {
				return encrypt((const void *) data, strlen(data), outsize);
			}

			/// @brief Decrypt data.
			/// @param data The data to decrypt.
			/// @param size The size of the input data.
			/// @param outsize The size of output data.
			/// @return A pointer to the decrypted data, release it with free().
			void * decrypt(const void *data, size_t size, size_t &outsize);

			/// @brief Generate a digest for data.
			/// @param data The data to digest.
			/// @param size The size of the input data.
			/// @param outsize The size of output data.
			/// @return A pointer to the digest, release it with free().
			void * digest(const void *data, size_t size, unsigned int &outsize);

			inline void * digest(const char *data, unsigned int &outsize) {
				return digest((const void *) data, strlen(data), outsize);
			}

			/// @brief Sign data.
			/// @param data The digest to sign.
			/// @param size The size of the digest data.
			/// @param outsize The size of output data.
			/// @return A pointer to the signature, release it with free().
			void * sign(const void *data, size_t size, size_t &outsize);

			/// @brief Verify data.
			/// @param sig The signature to verify.
			/// @param siglen The signature length.
			/// @param tbs The digest to verify.
			/// @param tbslen The digest length.
			/// @retval true If the signature is valid.
			/// @retval false If the signature is invalid.
			/// @throw Crypto::Exception If the verification fails.
			/// @throw std::runtime_error If the verification is not supported by the public key algorithm.
			bool verify(const void *sig, size_t siglen, const void *tbs, size_t tbslen);

		};

	}

}

namespace std {

	inline std::string to_string(const Udjat::Crypto::Key &key) {
		return key.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Crypto::Key &key) {
		return os << to_string(key);
	}

 }
