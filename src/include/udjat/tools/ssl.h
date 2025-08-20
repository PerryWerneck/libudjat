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

	namespace SSL {

		class BackEnd;

		class Exception : public Udjat::Exception {
		public:
			Exception(const char *msg);
			
			Exception(const std::string &msg) : Exception{msg.c_str()} {
			}
		};

		namespace Key {

			class UDJAT_API Private {
			private:
				EVP_PKEY *pkey = NULL;

				class Engine;
				std::shared_ptr<BackEnd> backend;

			public:

				constexpr Private() {
				}

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
				Private(const char *filename, const char *password, bool autogenerate = false);
				virtual ~Private();

				inline operator bool() const noexcept {
					return (bool) pkey;
				}

				inline operator EVP_PKEY *() const noexcept {
					return pkey;
				} 

				/// @brief Generates a new private key.
				/// @param mbits The size of the key in bits. Default is 2048 bits.
				/// @param mode The mode to generate the key, legacy, engine, provider or auto.
				/// If not specified, the mode will be taken from the configuration file.
				void generate(size_t mbits = 2048, const char *mode = nullptr);

				/// @brief Loads private key from file.
				/// @param filename The file where the private key is stored.
				/// @param password The password to protect the private key.
				void load(const char *filename, const char *passwd = NULL);

				/// @brief Save private key from file.
				/// @param filename The file where the private key will be stored.
				/// @param password The password to protect the private key.
				void save(const char *filename, const char *passwd = NULL);

				/// @brief Get singleton instance of the private key.
				// Private & getInstance();

				/// @brief Get string representation of the private key.
				/// @return The private key in PEM format.
				std::string to_string() const;

			};

		}

	}

}

namespace std {

	inline std::string to_string(const Udjat::SSL::Key::Private &key) {
		return key.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::SSL::Key::Private &key) {
		return os << to_string(key);
	}

 }
