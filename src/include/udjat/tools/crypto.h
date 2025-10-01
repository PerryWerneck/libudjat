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
#include <openssl/evp.h>
#include <udjat/tools/exception.h>
#include <memory>

namespace Udjat {

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
			void load(const char *filename, const char *passwd = NULL);

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
