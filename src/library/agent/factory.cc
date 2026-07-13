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
 #include <udjat/agent.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/script.h>
 #include <udjat/agent/percentage.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/http/error.h>
 #include <udjat/agent/state.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/url.h>
 #include <udjat/action.h>
 #include <udjat/tools/value.h>

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

		bool refresh(bool) override {
			
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

	bool Abstract::Agent::Factory::probe(const Properties &) const noexcept {
		return false;
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Factory::build(const Properties &props) {

		auto agent_name = props.get("name",PACKAGE_NAME);
		auto type = props["type"];
		if(type.empty()) {

			// No type, try probing the factories.

			for(const auto factory : Factories()) {

				if(!factory->probe(props)) {
					continue;
				}

				auto agent = factory->AgentFactory(props);
				if(agent) {
					return agent;
				}
			}

			// No factory recognize the node and I have no type, then, cant do anything.
			
			throw runtime_error(
				String{"Cant determine factory for agent '",props["name"].c_str(),"' at ",props.path()}
			);

		}

		//
		// Have type, use it
		//

		debug("Searching '",type.c_str(),"' in ",Factories().size()," available factories");
		for(const auto factory : Factories()) {

			debug("Checking for type '",type.c_str(),"' on factory '",factory->name,"'");
			if(strcasecmp(type.c_str(),factory->name)) {
				continue;
			}

			auto agent = factory->AgentFactory(props);
			if(agent) {
				debug("Got agent '",type.c_str(),"'")
				return agent;
			}

			Logger::String{"Agent '",props["name"].c_str()," rejected by factory '",factory->name,"'"}.trace();

		}

		// Try internal types
		if(strcasecmp(type.c_str(),"shell") == 0 || strcasecmp(type.c_str(),"script") == 0 || strcasecmp(type.c_str(),"shell-script") == 0) {

			/// @brief Agent keeping the value of a script return code.
			class Script : public Udjat::Agent<int32_t>, private Udjat::Script {
			public:
				Script(const Properties &props) : Udjat::Script{props} {
				}

				bool refresh(bool) override {
					return Udjat::Agent<int32_t>::set(
						(int32_t) Udjat::Script::run(*((Udjat::Agent<int32_t> *)this),false)
					);
				};

			};

			return make_shared<Script>(props);
		}

		static const struct
		{
			const char *type;
			function< std::shared_ptr<Abstract::Agent>(const Properties &props)> build;
		} builders[] = {

			{
				"int32",
				[](const Properties &props) {
					return make_shared<Udjat::Agent<int32_t>>(props);
				}
			},
			{
				"uint32",
				[](const Properties &props) {
					return make_shared<Udjat::Agent<uint32_t>>(props);
				}
			},
			{
				"integer",
				[](const Properties &props) {
					return make_shared<Udjat::Agent<int>>(props);
				}

			},
			{
				"boolean",
				[](const Properties &props) {
					return make_shared<Udjat::Agent<bool>>(props);
				}
			},
			{
				"string",
				[](const Properties &props) {
					return make_shared<Udjat::Agent<std::string>>(props);
				}
			},
			{
				"percentage",
				[](const Properties &props) {
					return make_shared<Udjat::Agent<Percentage>>(props);
				}
			},
			{
				"%",
				[](const Properties &props) {
					return make_shared<Udjat::Agent<Percentage>>(props);
				}
			},		
			{
				"url",
				[](const Properties &props) {

					/// @brief Agent keeping the value of url status code.
					class Url : public Udjat::Agent<int32_t> {
					private:
						const char *url;
						HTTP::Method method;

					public:
						Url(const Properties &props) 
							: Udjat::Agent<int32_t>(props), url{props["url"].as_quark()},method{HTTP::MethodFactory(props,"method","head")}  {

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

						bool refresh(bool) override {
							return set((int32_t) Udjat::URL{this->url}.test(method));
						};

					};

					return make_shared<Url>(props);
				}
			},

		};

		for(auto builder : builders) {
			if(!strcasecmp(type.c_str(),builder.type)) {
				Logger::String{"Building agent using internal type '",type,"'"}.trace(agent_name.c_str());
				return builder.build(props);
			}
		}

		if(strcasecmp(type.c_str(),"random") == 0 || strcasecmp(type.c_str(),"randomvalue") == 0) {

			/// @brief Agent generating a random value.
			class RandomValue : public Udjat::Agent<unsigned int> {
			public:
			private:
				unsigned int limit = 5;

			public:
				RandomValue(const Properties &props) : Agent<unsigned int>(props) {
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

				bool refresh(bool) override {
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

			Logger::String{"Building random value agent"}.trace(agent_name.c_str());
			return make_shared<RandomValue>(props);

		}

		// Try actions
		try {

			std::shared_ptr<Action> action = Action::Factory::build(props);

			Logger::String{"Building action based agent"}.trace(agent_name.c_str());

			switch(Value::TypeFactory(props,"value-type","int")) {
			case Value::String:
				return make_shared<ActionAgent<string>>(props,action);

			case Value::Signed:
				return make_shared<ActionAgent<int>>(props,action);

			case Value::Unsigned:
				return make_shared<ActionAgent<unsigned int>>(props,action);

			case Value::Real:
			case Value::Fraction:
				return make_shared<ActionAgent<double>>(props,action);

			case Value::Boolean:
				return make_shared<ActionAgent<bool>>(props,action);

			default:
				throw logic_error("Invalid attribute: value-type");
			}
 
		} catch(...) {

			// Ignore exceptions from action factory

		}

		throw runtime_error(
			String{"Cant find a valid factory for agent '",agent_name.c_str(),"' type '",type,"' at ",props.path()}
		);

	}

 }
