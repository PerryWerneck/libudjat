#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <iostream>
	#include <udjat/agent.h>
	#include <unordered_map>
	#include <udjat/tools/atom.h>
	#include <pugixml.hpp>

	#ifdef HAVE_UNISTD_H
		#include <unistd.h>
	#endif // HAVE_UNISTD_H

	using namespace std;

	namespace Udjat {

		class Abstract::Agent::Controller : public Abstract::Agent {
		public:
			Controller();
			~Controller();

			void refresh() override;

		};

		class Abstract::Agent::Factory {
		private:
			static std::recursive_mutex guard;

			class Method {
			private:
				std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> value;

			public:
				Method(std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> m) : value(m) {}

				std::shared_ptr<Abstract::Agent> create(Abstract::Agent &parent, const pugi::xml_node &node) {
					return this->value(parent,node);
				}

			};

			std::unordered_map<Udjat::Atom, Method> methods;
			Factory();

		public:
			static Factory & getInstance();
			~Factory();

			void insert(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method);

			void load(Abstract::Agent &parent,const pugi::xml_node &node);

		};


	}



#endif // PRIVATE_H_INCLUDED
