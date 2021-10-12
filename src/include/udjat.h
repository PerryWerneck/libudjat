
#ifndef UDJAT_H_INCLUDED

	#define UDJAT_H_INCLUDED

	#include <udjat/defs.h>
	#include <string>
	#include <functional>

	namespace Udjat {

		/// @brief Expands ${key} tags.
		/// @param str	String to expand.
		/// @param exec Method to expand (Returns "${}" to disable expansion).
		void UDJAT_API expand(std::string &str, std::function<std::string (const char *key)> exec);

	}

#endif // UDJAT_H_INCLUDED
