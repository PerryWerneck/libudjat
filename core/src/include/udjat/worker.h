
#pragma once

#include <udjat/defs.h>
#include <udjat/request.h>
#include <udjat/tools/quark.h>

namespace Udjat {

	class UDJAT_API Worker {
	private:
		Quark name;
		class Controller;

	public:
		Worker(const Quark &name);

		inline size_t hash() const {
			return name.hash();
		}

		inline const char * c_str() const {
			return name.c_str();
		}

		virtual ~Worker();

		virtual void work(const Request &request, Response &response) = 0;

	};

}
