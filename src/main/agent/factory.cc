/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/factory.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/http/error.h>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Factory(const Abstract::Object &parent, const pugi::xml_node &node) {
		return Factory(node.attribute("type").as_string(node.name()),parent,node);
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Factory(const char *type, const Abstract::Object &parent, const pugi::xml_node &node) {

		// Check for standard factory.
		{
			auto factory = Factory::find(type);
			if(factory) {
				auto agent = factory->AgentFactory(parent,node);
				if(agent) {
					return agent;
				}
			}
		}

		// Check for module based factory.
		{
			auto module = Module::find(type);
			if(module) {
				auto agent = module->AgentFactory(parent,node);
				if(agent) {
					return agent;
				}
			}
		}

		// Check for internal factories.
		{
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

						/// @brief Agent keeping the value of script return code.
						class Script : public Udjat::Agent<int32_t> {
						private:
							const char *cmdline;
							Logger::Level out = Logger::Info;
							Logger::Level err = Logger::Error;

						public:
							Script(const pugi::xml_node &node)
								: Udjat::Agent<int32_t>(node),
									out{Logger::LevelFactory(node,"stdout","info")},
									err{Logger::LevelFactory(node,"stderr","error")}
							{
								cmdline = Quark(node,"cmdline","",false).c_str();
								if(!cmdline) {
									throw runtime_error("Required attribute 'cmdline' is missing");
								}
							}

							bool refresh(bool UDJAT_UNUSED(ondemand)) {

								// Execute script, save return code.

								int32_t value = -1;

								try {

									value = SubProcess::run(this,cmdline,out,err);

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

				{
					"url",
					[](const pugi::xml_node &node) {

						/// @brief Agent keeping the value of script return code.
						class Url : public Udjat::Agent<int32_t> {
						private:
							const char *url;
							HTTP::Method method;

						public:
							Url(const pugi::xml_node &node) : Udjat::Agent<int32_t>(node), url{Quark(node,"url","",false).c_str()},method{HTTP::MethodFactory(node.attribute("method").as_string("head"))}  {

								if(!(url && *url)) {
									throw runtime_error("Required attribute 'url' is missing");
								}

							}

							std::shared_ptr<Abstract::State> computeState() override {

								std::shared_ptr<Abstract::State> state = Udjat::Agent<int32_t>::computeState();

								if(!state) {
									state = HTTP::Error::StateFactory(this->get());
								}

								if(!state) {
									state = Abstract::Agent::computeState();
								}

								return state;

							}

							bool refresh(bool UDJAT_UNUSED(ondemand)) {
								return set((int32_t) Udjat::URL{this->url}.test(method));
							};

						};

						return make_shared<Url>(node);
					}
				},

			};

			for(auto builder : builders) {

				if(!strcasecmp(type,builder.type)) {
					return builder.build(node);
				}

			}

		}

		return std::shared_ptr<Abstract::Agent>();
	}

 }
