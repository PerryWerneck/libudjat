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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/crypto.h>
 #include <udjat/tools/file/text.h>
 #include <memory>

 #include <openssl/bio.h>
 #include <openssl/evp.h>

 namespace Udjat {

	/// @brief Abstract base class for SSL key backends.
	/// This class provides a common interface for different SSL key backends,
	/// such as legacy, engine, and provider.
	/// It allows for loading, saving, and generating SSL keys in a uniform way.
	/// @note This class is not intended to be used directly, but rather through its derived
	/// classes that implement the specific backend logic.
	/// @see SSL::Key::BackEnd::Factory
	class UDJAT_PRIVATE Crypto::BackEnd {
	protected:
		EVP_PKEY *pkey = NULL;

		/// @brief The type of the backend.
		/// On the provider and engine backends this is the name of the provider/engine loaded, usually tpm2tss.
		String type;

		BackEnd(const char *name, const char *type);

	public:

		BackEnd(BackEnd &) = delete;
		BackEnd(BackEnd *) = delete;
		BackEnd(const BackEnd &) = delete;
		BackEnd(const BackEnd *) = delete;

		static std::shared_ptr<Crypto::BackEnd> Factory(Udjat::String name);

		virtual ~BackEnd();

		inline operator bool() const noexcept {
			return (bool) pkey;
		}

		inline operator EVP_PKEY *() const noexcept {
			return pkey;
		}

		virtual void save_private(const char *filename, const char *password);
		virtual void save_public(const char *filename);
		virtual void load(const char *filename, const char *password);

		/// @brief Encrypt data.
		/// @param data The data to encrypt.
		/// @param size The size of the input data.
		/// @param outsize The size of output data.
		/// @return A pointer to the encrypted data, release it with free().
		virtual void * encrypt(const void *data, size_t size, size_t &outsize);

		/// @brief Decrypt data.
		/// @param data The data to decrypt.
		/// @param size The size of the input data.
		/// @param outsize The size of output data.
		/// @return A pointer to the decrypted data, release it with free().
		virtual void * decrypt(const void *data, size_t size, size_t &outsize);

		/// @brief Generate a digest for data.
		/// @param data The data to digest.
		/// @param size The size of the input data.
		/// @param outsize The size of output data.
		/// @return A pointer to the digest, release it with free().
		virtual void * digest(const void *data, size_t size, unsigned int &outsize);

		/// @brief Sign data.
		/// @param data The data to sign.
		/// @param size The size of the input data.
		/// @param outsize The size of output data.
		/// @return A pointer to the output data, release it with free().
		virtual void * sign(const void *data, size_t size, size_t &outsize);

		/// @brief Verify data.
		/// @param sig The signature to verify.
		/// @param siglen The signature length.
		/// @param tbs The data to verify.
		/// @param tbslen The data length.
		/// @retval true If the signature is valid.
		/// @retval false If the signature is invalid.
		/// @throw Crypto::Exception If the verification fails.
		/// @throw std::runtime_error If the verification is not supported by the public key algorithm.
		virtual bool verify(const void *sig, size_t siglen, const void *tbs, size_t tbslen);

		void unload();

		virtual std::string get_private();
		virtual std::string get_public();

		/// @brief Generate key using subprocess.
		/// @param filename The file where the private key will be stored.
		/// @param passwd The password to protect the private key.
		/// @param mbits The size of the key in bits.
		/// @return true if the subprocess was run, false if no subprocess is defined.
		bool subproc(const char *filename, const char *passwd, size_t mbits);

		/// @brief Generate key using backend internal functions.
		/// @param filename The file where the private key will be stored.
		/// @param passwd The password to protect the private key.
		/// @param mbits The size of the key in bits.
		virtual void generate(const char *filename, const char *password, size_t mbits = 2048) = 0;

		static std::shared_ptr<Crypto::BackEnd> LegacyFactory();

#if defined(HAVE_OPENSSL_ENGINE)
		static std::shared_ptr<Crypto::BackEnd> EngineFactory();
#endif // HAVE_OPENSSL_ENGINE

#if defined(HAVE_OPENSSL_PROVIDER)
		static std::shared_ptr<Crypto::BackEnd> ProviderFactory();
#endif // HAVE_OPENSSL_PROVIDER

	};

 }