
#ifndef UDJAT_H_INCLUDED

	#define UDJAT_H_INCLUDED

	#include <udjat/defs.h>

	namespace Udjat {

		/// @brief Utility method, starts all modules, run mainloop until interrupted the stops all modules.
		void UDJAT_API run() noexcept;

		void UDJAT_API start() noexcept;
		void UDJAT_API stop() noexcept;
		void UDJAT_API reload() noexcept;
	}

#endif // UDJAT_H_INCLUDED
