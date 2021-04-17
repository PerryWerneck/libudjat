
#pragma once

#include <udjat/defs.h>
#include <udjat/request.h>
#include <udjat/tools/quark.h>

namespace Udjat {

	class UDJAT_API Worker {
	private:
		Quark name;
		class Controller;
		friend class Controller;

	protected:

		/// @brief True if the worker is active.
		bool active;

		/// @brief Information about the worker module.
		const ModuleInfo *info;

	public:
		Worker(const Quark &name);

		static void work(const char *name, const Request &request, Response &response);

		inline size_t hash() const {
			return name.hash();
		}

		inline const char * c_str() const {
			return name.c_str();
		}

		virtual ~Worker();

		virtual void work(const Request &request, Response &response) const = 0;

	};

}
