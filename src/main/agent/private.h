#pragma once

#include <config.h>
#include <iostream>
#include <udjat/agent.h>
#include <unordered_map>
#include <udjat/tools/quark.h>
#include <pugixml.hpp>
#include <udjat/worker.h>
#include <udjat/module.h>
#include <udjat/factory.h>
#include <udjat/tools/mainloop.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif // HAVE_UNISTD_H

using namespace std;

namespace Udjat {

	std::shared_ptr<Abstract::Agent> getDefaultRootAgent();

	class Abstract::Agent::Controller : private Worker, Factory, MainLoop::Service {
	private:

		std::shared_ptr<Abstract::Agent> root;

		Controller(const Controller &) = delete;
		Controller(const Controller *) = delete;
		Controller();

		void onTimer(time_t tm) noexcept;

	public:
		~Controller();

		static Controller & getInstance();

		void set(std::shared_ptr<Abstract::Agent> root);

		std::shared_ptr<Abstract::Agent> get() const;
		std::shared_ptr<Abstract::Agent> find(const char *path) const;

		bool get(Request &request, Response &response) const override;
		bool head(Request &request, Response &response) const override;

		bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const override;

		void start() noexcept override;
		void stop() noexcept override;

	};

}
