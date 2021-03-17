
#pragma once

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <udjat/agent.h>

	namespace Udjat {

		/// @brief Udjat module.
		class UDJAT_API Module {
		private:
			class Controller;
			friend class Controller;

			int handle;
			Quark name;

		public:
			Module();
			virtual ~Module();

			/// @brief Root agent is changing.
			virtual void set(std::shared_ptr<Abstract::Agent> agent);

			/// @brief Start module.
			virtual void start();

			/// @brief Reload module.
			virtual void reload();

			/// @brief Stop module.
			virtual void stop();

		};

	}

	extern "C" {

		/// @brief Initialize module.
		/// @return Module controller.
		Module * UDJAT_API udjat_module_init(void);

	}
