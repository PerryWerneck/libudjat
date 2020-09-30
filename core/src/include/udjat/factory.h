#ifndef UDJAT_FACTORY_H_INCLUDED

	#define UDJAT_FACTORY_H_INCLUDED

	#include <udjat/defs.h>

	namespace Udjat {

		namespace Factory {

			class Controller;

			/// @brief Load xml document.
			void load(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_document &doc);

			/// @brief Verify if the name is a valid factory name.
			const char * validate_name(const char *name);

			/// @brief Register an agent factory.
			void insert(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> factory);

			/// @brief Register a node factory.
			void insert(const char *name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> factory);

		}

	}


#endif // UDJAT_FACTORY_H_INCLUDED
