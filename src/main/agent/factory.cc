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
 #include <udjat/tools/factory.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/http/error.h>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Factory(const Abstract::Object &parent, const XML::Node &node) {

		std::shared_ptr<Abstract::Agent> agent;

		// Check for standard factories.
		if(Udjat::Factory::for_each([&node, &agent, &parent](Udjat::Factory &factory){

			if(factory == node.attribute("type").as_string("default")) {
				agent = factory.AgentFactory(parent,node);
				return (bool) agent;
			}
			return false;

		})) {
			return agent;
		}

		// Check for internal factories.
		{
			const char *type = node.attribute("type").as_string();
			static const struct
			{
				const char *type;
				function< std::shared_ptr<Abstract::Agent>(const XML::Node &node)> build;
			} builders[] = {

				{
					"int32",
					[](const XML::Node &node) {
						return make_shared<Udjat::Agent<int32_t>>(node);
					}
				},
				{
					"uint32",
					[](const XML::Node &node) {
						return make_shared<Udjat::Agent<uint32_t>>(node);
					}
				},
				{
					"integer",
					[](const XML::Node &node) {
						return make_shared<Udjat::Agent<int>>(node);
					}

				},
				{
					"boolean",
					[](const XML::Node &node) {
						return make_shared<Udjat::Agent<bool>>(node);
					}
				},
				{
					"string",
					[](const XML::Node &node) {
						return make_shared<Udjat::Agent<std::string>>(node);
					}
				},
				{
					"script",
					[](const XML::Node &node) {

						/// @brief Agent keeping the value of script return code.
						class Script : public Udjat::Agent<int32_t> {
						private:
							const char *cmdline;
							Logger::Level out = Logger::Info;
							Logger::Level err = Logger::Error;

						public:
							Script(const XML::Node &node)
								: Udjat::Agent<int32_t>(node),
									out{Logger::LevelFactory(node,"stdout","info")},
									err{Logger::LevelFactory(node,"stderr","error")}
							{
								cmdline = Quark(node,"cmdline","").c_str();
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
					[](const XML::Node &node) {

						/// @brief Agent keeping the value of script return code.
						class Url : public Udjat::Agent<int32_t> {
						private:
							const char *url;
							HTTP::Method method;

						public:
							Url(const XML::Node &node) : Udjat::Agent<int32_t>(node), url{Quark(node,"url","").c_str()},method{HTTP::MethodFactory(node.attribute("method").as_string("head"))}  {

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
					agent = builder.build(node);
					if(agent) {
						break;
					}
				}

			}

		}

		return agent;
	}

 }
