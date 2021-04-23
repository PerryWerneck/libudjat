
#ifndef UDJAT_H_INCLUDED

	#define UDJAT_H_INCLUDED

	#include <udjat/defs.h>
	#include <string>
	#include <functional>

	namespace Udjat {

		/// @brief Utility method, starts all modules, run mainloop until interrupted then stops all modules.
		void UDJAT_API run() noexcept;

		/// @brief Start all modules.
		void UDJAT_API start() noexcept;

		/// @brief Stop all modules.
		void UDJAT_API stop() noexcept;

		/// @brief Reload all modules.
		void UDJAT_API reload() noexcept;

		/// @brief Expands ${key} tags.
		/// @param str	String to expand.
		/// @param exec Method to expand (Returns "${}" to disable expansion).
		void UDJAT_API expand(std::string &str, std::function<std::string (const char *key)> exec);

	}

#endif // UDJAT_H_INCLUDED
