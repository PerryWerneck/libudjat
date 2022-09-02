/**
 * @file src/core/agent/controller.cc
 *
 * @brief Implements the root agent.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include "private.h"
 #include <udjat/tools/subprocess.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::AgentFactory(const Abstract::Object &parent, const pugi::xml_node &node) const {

		static const struct
		{
			const char *type;
			function< std::shared_ptr<Abstract::Agent>(const pugi::xml_node &node)> build;
		} builders[] = {

			{
				"int32",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<int32_t>>(node);
				}
			},
			{
				"uint32",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<uint32_t>>(node);
				}
			},
			{
				"integer",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<int>>(node);
				}

			},
			{
				"boolean",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<bool>>(node);
				}
			},
			{
				"string",
				[](const pugi::xml_node &node) {
					return make_shared<Udjat::Agent<std::string>>(node);
				}
			},
			{
				"script",
				[](const pugi::xml_node &node) {

					class Script : public Udjat::Agent<int32_t> {
					private:
						const char *cmdline;

					public:
						Script(const pugi::xml_node &node) : Udjat::Agent<int32_t>(node) {
							cmdline = Quark(node,"cmdline","",false).c_str();
							if(!cmdline) {
								throw runtime_error("Required attribute 'cmdline' is missing");
							}
						}

						bool refresh(bool UDJAT_UNUSED(ondemand)) {

							int32_t value = -1;

							try {

								value = SubProcess::run(cmdline);

							} catch(const std::exception &e) {

								value = -1;
								error() << "Error '" << e.what() << "' running script" << endl;

							} catch(...) {

								error() << "Unexpected error running script" << endl;
								value = -1;

							}

							return set(value);
						};

					};

					return make_shared<Script>(node);
				}
			},

		};

		const char *type = node.attribute("type").as_string(builders[0].type);

		if(type && *type) {

			// Check for module factory.
			Factory *factory(Factory::find(type));
			if(factory) {
				auto agent = factory->AgentFactory(parent,node);
				if(agent) {
					return agent;
				}
			}

			// Check for internal builders.
			for(auto builder : builders) {

				if(!strcasecmp(type,builder.type)) {
					auto agent = builder.build(node);
					return agent;
				}

			}

		}

		return Factory::AgentFactory(parent,node);
	}

}

