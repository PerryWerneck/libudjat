/*
 *
 * Copyright (C) <2020> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @file
 *
 * @brief
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 *
 */

#ifndef UDJAT_DEFS_H_INCLUDED

	#define UDJAT_DEFS_H_INCLUDED 1

	#include <json/value.h>

#ifdef _WIN32

	#pragma GCC diagnostic ignored "-Wpedantic"

	#define _WIN32_WINNT 0x0600

	#include <mswsock.h>
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>

	#define MSG_NOSIGNAL 0

#endif // WIN32

	/// @brief Macro para informar uso de argumentos printf.
	#define AS_PRINTF(a,b) __attribute__((format(printf, a, b)))

	// Branch prediction macros
	//
	// Referências:
	//
	// https://stackoverflow.com/questions/1668013/can-likely-unlikely-macros-be-used-in-user-space-code
	// http://www.geeksforgeeks.org/branch-prediction-macros-in-gcc/
	//
	#ifdef __GNUC__
		#define likely(x)       __builtin_expect(!!(x), 1)
		#define unlikely(x)     __builtin_expect(!!(x), 0)
	#else
		#define likely(x)       (x)
		#define unlikely(x)     (x)
	#endif

	/**
	 * @brief Macro para informar que um parâmetro não é utilizado.
	 *
	 * Macro que informa ao compilador que um parâmetro passado à função não é usado;
	 * útil para evitar o warning "unused parameter" em caso de callbacks que possuem
	 * argumentos pré-definidos.
	 *
	 * <http://stackoverflow.com/questions/3417837/what-is-the-best-way-to-supress-unused-variable-x-warning>
	 * <http://sourcefrog.net/weblog/software/languages/C/unused.html>
	 *
	 */
	#ifdef UNUSED

	#elif defined(__GNUC__)

		#define UDJAT_UNUSED(x) __attribute__((unused)) x
		#define UDJAT_DEPRECATED(func) func __attribute__ ((deprecated))

	#elif defined(_WIN32)

		#define UDJAT_UNUSED(x) x
		#define UDJAT_DEPRECATED(func) __declspec(deprecated) func

	#elif defined(__LCLINT__)

		#define UDJAT_UNUSED(x) /*@unused@*/ x
		#define UDJAT_DEPRECATED(func) func

	#else

		#define UDJAT_UNUSED(x) x
		#define UDJAT_DEPRECATED(func) func

	#endif

	// Declara símbolos exportados pela biblioteca.
	#if defined(_WIN32)

		#define UDJAT_API	__declspec (dllexport)
		#define UDJAT_PRIVATE

	#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)

		#define UDJAT_API
		#define UDJAT_PRIVATE

	#else

		#define UDJAT_API	__attribute__((visibility("default")))
		#define UDJAT_PRIVATE	__attribute__((visibility("hidden")))

	#endif

	/// @brief Obtém nome de variável como string.
	#define STRINGIZE(x) #x

	/**symbol.c_str()
	 * @brief Converte valor passado ao gcc via linha de comando em string.
	 *
	 * Macro usada para converter valores passados ao gcc via definições de
	 * linha de comando em uma string. Geralmente usado para converter os
	 * nomes de diretórios ajustados pelo ./configure.
	 *
	 * <http://stackoverflow.com/questions/2410976/how-to-define-a-string-in-gcc-command-line>
	 */
	#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

	/// @brief Obtém o número de elementos de um array.
	#define N_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))

	namespace Udjat {

		/// @brief Module information data.
		struct UDJAT_API ModuleInfo {

			/// @brief The module name.
			const char *name;

			/// @brief The module description.
			const char *description;

			/// @brief The module version.
			const char *version;

			/// @brief The bugreport address.
			const char *bugreport;

			/// @brief The package URL.
			const char *url;

			/// @brief The file path.
			const char *path = nullptr;

			constexpr ModuleInfo(const char *n = "", const char *d = "", const char *v = "", const char *u="", const char *b= "") :
				name(n), description(d), version(v), bugreport(b), url(u) { }

			// ModuleInfo(const char *args, ...) __attribute__ ((sentinel));

			Json::Value & get(Json::Value &value) const;

		};

		/// @brief Abstract objects.
		namespace Abstract {

			class Agent;
			class State;

		}

		class Alert;

	}

#endif // COMPONENTS_DEFS_H_INCLUDED

