/**
 *   base64.cc - Modified version by Perry Werneck <perry.werneck@gmail.com>
 *
 *   Copyright (C) 2004-2008 René Nyffenegger
 *
 *   This source code is provided 'as-is', without any express or implied
 *   warranty. In no event will the author be held liable for any damages
 *   arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose,
 *   including commercial applications, and to alter it and redistribute it
 *   freely, subject to the following restrictions:
 *
 *   1. The origin of this source code must not be misrepresented; you must not
 *      claim that you wrote the original source code. If you use this source code
 *      in a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original source code.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 *
 *   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
 *
 *
 * @file base64.cc
 *
 * @brief
 *
 * @author René Nyffenegger
 *
 */

#pragma once

#include <udjat/defs.h>
#include <string>
#include <cstring>

namespace Udjat {

	namespace Base64 {

		std::string UDJAT_API encode(const unsigned char * bytes_to_encode, size_t in_len) noexcept;

		inline std::string encode(const char *str) noexcept {
			return encode((const unsigned char *) str,strlen(str));
		}

		inline std::string encode(const std::string &str) noexcept {
			return encode((const unsigned char *) str.c_str(),str.size());
		}

		ssize_t UDJAT_API decode(const unsigned char *encoded_string, unsigned char * decoded_string, size_t maxlen) noexcept;

	}

}
