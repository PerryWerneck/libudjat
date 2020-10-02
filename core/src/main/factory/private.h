
#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <udjat/defs.h>
	#include <udjat/agent.h>
	#include <udjat/tools/atom.h>
	#include <udjat/factory.h>
	#include <mutex>
	#include <iostream>
	#include <unordered_map>

	using namespace std;

	namespace Udjat {

		class Factory::Controller {
		private:

			// Hash method
			class Hash {
			public:
				inline size_t operator() (const char *a) const {

					// https://stackoverflow.com/questions/7666509/hash-function-for-string
					size_t value = 5381;

					for(const char *ptr = a; *ptr; ptr++) {
						value = ((value << 5) + value) + tolower(*ptr);
					}

					return value;
				}
			};

			// Equal method
			class Equal {
			public:
				inline bool operator() (const char * a, const char * b) const {
					return strcasecmp(a,b) == 0;
				}
			};

			class Agent {
			private:
				std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> value;

			public:
				Agent(std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> m) : value(m) {}

				std::shared_ptr<Abstract::Agent> create(Abstract::Agent &parent, const pugi::xml_node &node) {
					return this->value(parent,node);
				}

			};

			std::unordered_map<const char *, Agent, Hash, Equal> agents;

			class Node {
			private:
				std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> value;

			public:
				Node(std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> m) : value(m) {}

				void apply(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node) {
					this->value(agent,node);
				}

			};

			std::unordered_map<const char *, Node, Hash, Equal> nodes;

			static std::recursive_mutex guard;

			Controller();

		public:
			~Controller();
			static Controller & getInstance();

			/// @brief Insert agent factory method.
			void insert(const Atom &name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method);

			/// @brief Insert Node factory method.
			void insert(const Atom &name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> factory);

			/// @brief Load agent children from xml definition.
			void load(std::shared_ptr<Abstract::Agent> parent, const pugi::xml_node &node);

		};

	}


#endif // PRIVATE_H_INCLUDED
