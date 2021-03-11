#pragma once

#include <config.h>
#include <iostream>
#include <udjat/agent.h>
#include <unordered_map>
#include <udjat/tools/quark.h>
#include <pugixml.hpp>
#include <udjat/worker.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif // HAVE_UNISTD_H

using namespace std;

namespace Udjat {

	const char * check_for_reserved_name(const char *);

	class Abstract::Agent::Controller : private Worker {
	private:

		std::shared_ptr<Abstract::Agent> root;

		Controller();

	public:
		static Controller & getInstance();

		void set(std::shared_ptr<Abstract::Agent> root);

		std::shared_ptr<Abstract::Agent> get() const;
		std::shared_ptr<Abstract::Agent> find(const char *path) const;

		void work(const Request &request, Response &response) const override;

	};

}
