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
 #include <udjat/tools/ssl.h>
 #include <udjat/tools/file/text.h>
 #include <memory>

 #include <openssl/bio.h>
 #include <openssl/evp.h>

 #ifdef HAVE_OPENSSL_ENGINE
 #define OPENSSL_SUPPRESS_DEPRECATED
 #include <openssl/engine.h>
 #endif // HAVE_OPENSSL_ENGINE

 #ifdef HAVE_TPM2_TSS_ENGINE_H
 #include <tpm2-tss-engine.h>
 #endif // HAVE_TPM2_TSS_ENGINE_H

 #ifdef HAVE_OPENSSL_PROVIDER
 #include <openssl/provider.h>
 #endif // HAVE_OPENSSL_PROVIDER


 using BIO_PTR = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
 using CTX_PTR = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
 using BIGNUM_PTR = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

 namespace Udjat {

	/// @brief Abstract base class for SSL key backends.
	/// This class provides a common interface for different SSL key backends,
	/// such as legacy, engine, and provider.
	/// It allows for loading, saving, and generating SSL keys in a uniform way.
	/// @note This class is not intended to be used directly, but rather through its derived
	/// classes that implement the specific backend logic.
	/// @see SSL::Key::BackEnd::Factory
	class UDJAT_PRIVATE SSL::Key::BackEnd {
	public:
		const char *type; ///< The type of the backend for configuration purposes.

		static std::shared_ptr<SSL::Key::BackEnd> Factory(Udjat::String name);

		constexpr BackEnd(const char *t) : type{t} {
		}

		virtual ~BackEnd() {
		}

		virtual void save_private(EVP_PKEY *pkey, const char *filename, const char *password);
		virtual void save_public(EVP_PKEY *pkey, const char *filename);
		virtual EVP_PKEY * load(const File::Text &file, const char *filename, const char *password);
		virtual std::string get_private(EVP_PKEY *pkey, const char *);
		virtual std::string get_public(EVP_PKEY *pkey);
		virtual EVP_PKEY * generate(const char *filename, const char *password, size_t mbits = 2048) = 0;

	};

	

 }