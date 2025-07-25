/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

/**
 * @brief Implements the agent factory.
 * @author perry.werneck@gmail.com
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/agent/state.h>
 #include <udjat/agent.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/http/error.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/script.h>
 #include <udjat/tools/value.h>
 #include <udjat/agent/percentage.h>

 #include <cstring>
 #include <list>
 #include <memory>
 
 using namespace std;

 namespace Udjat {

	/// @brief Agent using the action engine to get value.
	template <typename T>
	class ActionAgent : public Udjat::Agent<T> {
	private:
		std::shared_ptr<Action> action;
		const char *valuename;	///< @brief name of the field for agent value.

	public:
		ActionAgent(const XML::Node &node, std::shared_ptr<Action> a) 
		: Udjat::Agent<T>{node}, action{a}, valuename{String{node,"value-from","value"}.as_quark()} {
		}

		bool refresh() override {
			
			Request request;
			Response response;

			this->getProperties(request);

			T val = this->get();

			request[valuename] = val;
			response[valuename] = val;

			action->call(request,response,true);

			response[valuename].get(val);
			return this->set(val);
		}

	};

	static Container<Abstract::Agent::Factory> & Factories() {
		static Container<Abstract::Agent::Factory> instance;
		return instance;
	}

	Abstract::Agent::Factory::Factory(const char *n) : name{n} {
		Factories().push_back(this);
	}

	Abstract::Agent::Factory::~Factory() {
		Factories().remove(this);
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Factory::build(const Abstract::Agent &parent, const XML::Node &node) {

		const char *type = node.attribute("type").as_string("default");

		for(const auto factory : Factories()) {

			if(strcasecmp(type,factory->name)) {
				continue;
			}

			auto agent = factory->AgentFactory(parent,node);
			if(agent) {
				debug("Got agent '",type,"'")
				return agent;
			}

		}

		// Try internal types
		if(strcasecmp(type,"shell") == 0 || strcasecmp(type,"script") == 0 || strcasecmp(type,"shell-script") == 0) {

			/// @brief Agent keeping the value of script return code.
			class Script : public Udjat::Agent<int32_t>, private Udjat::Script {
			public:
				Script(const XML::Node &node) : Udjat::Script{node} {
				}

				bool refresh(bool) {
					return Udjat::Agent<int32_t>::set(
						(int32_t) Udjat::Script::run(*((Udjat::Agent<int32_t> *)this),false)
					);
				};

			};

			return make_shared<Script>(node);
		}

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
				"percentage",
				[](const XML::Node &node) {
					return make_shared<Udjat::Agent<Percentage>>(node);
				}
			},
			{
				"%",
				[](const XML::Node &node) {
					return make_shared<Udjat::Agent<Percentage>>(node);
				}
			},		
			{
				"url",
				[](const XML::Node &node) {

					/// @brief Agent keeping the value of url status code.
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

							int32_t value = Udjat::Agent<int32_t>::get();

							for(auto state : states) {
								if(state->compare(value))
									return state;
							}

							std::shared_ptr<Abstract::State> state = HTTP::Error::StateFactory(value);
							if(state) {
								return state;
							}


							return Abstract::Agent::computeState();

						}

						bool refresh(bool) {
							return set((int32_t) Udjat::URL{this->url}.test(method));
						};

					};

					return make_shared<Url>(node);
				}
			},

		};

		for(auto builder : builders) {
			if(!strcasecmp(type,builder.type)) {
				Logger::String{"Building agent using internal type '",type,"'"}.trace(node.attribute("name").as_string(PACKAGE_NAME));
				return builder.build(node);
			}
		}

		if(strcasecmp(type,"random") == 0 || strcasecmp(type,"randomvalue") == 0) {

			/// @brief Agent generating a random value.
			class RandomValue : public Udjat::Agent<unsigned int> {
			public:
			private:
				unsigned int limit = 5;

			public:
				RandomValue(const XML::Node &node) : Agent<unsigned int>(node) {
				}

				std::shared_ptr<Abstract::State> computeState() override {
					for(auto state : states) {
						if(state->compare(get())) {
							return state;
						}
					}
					Logger::String{"No state matched with value ",get()}.trace(name());
					return Abstract::Agent::computeState();
				}

				bool refresh() override {
					unsigned int last = get();
					unsigned int value = ((unsigned int) rand()) % limit;
					if(value == last) {
						Logger::String{"Current value stayed the same: ",value}.info(name());
						return false;
					}
					Logger::String{"Value changed from ",last," to ",value}.info(name());
					return set(value);
				}

				void start() override {
					Agent<unsigned int>::start( ((unsigned int) rand()) % limit );
				}
				
			};

			Logger::String{"Building random value agent"}.trace(node.attribute("name").as_string(PACKAGE_NAME));
			return make_shared<RandomValue>(node);

		}

		// Try actions
		{
			std::shared_ptr<Action> action = Action::Factory::build(node,false);
			if(action) {

				Logger::String{"Building action based agent"}.trace(node.attribute("name").as_string(PACKAGE_NAME));

				switch(Value::TypeFactory(node,"value-type","int")) {
				case Value::String:
					return make_shared<ActionAgent<string>>(node,action);

				case Value::Signed:
					return make_shared<ActionAgent<int>>(node,action);

				case Value::Unsigned:
					return make_shared<ActionAgent<unsigned int>>(node,action);

				case Value::Real:
				case Value::Fraction:
					return make_shared<ActionAgent<double>>(node,action);

				case Value::Boolean:
					return make_shared<ActionAgent<bool>>(node,action);

				default:
					throw logic_error("Invalid attribute: value-type");
				}

			}

		}

		throw runtime_error(
			String{"Cant find a valid factory for agent type '",type,"'"}
		);

	}

 }
