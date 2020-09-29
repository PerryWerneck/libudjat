
#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/defs.h>
	#include <udjat/agent.h>
	#include <udjat/factory.h>
	#include <pugixml.hpp>
	#include <mutex>
	#include <iostream>

	using namespace std;

	namespace Udjat {

		class Factory::Controller {
		private:

			class Agent {
			private:
				std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> value;

			public:
				Agent(std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> m) : value(m) {}

				std::shared_ptr<Abstract::Agent> create(Abstract::Agent &parent, const pugi::xml_node &node) {
					return this->value(parent,node);
				}

			};

			std::unordered_map<Udjat::Atom, Agent> agents;

			static std::recursive_mutex guard;

			Controller();

		public:
			~Controller();
			static Controller & getInstance();

			/// @brief Insert agent factory method.
			void insert(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method);

			/// @brief Load agent children from xml definition.
			void load(Abstract::Agent &parent, const pugi::xml_node &node);

		};

	}


#endif // PRIVATE_H_INCLUDED
