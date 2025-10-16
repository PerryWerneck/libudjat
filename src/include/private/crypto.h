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

 // using BIO_PTR = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
 // using CTX_PTR = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
 // using BIGNUM_PTR = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

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