#ifndef UDJAT_FACTORY_H_INCLUDED

	#define UDJAT_FACTORY_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>

	namespace Udjat {

		namespace Factory {

			class Controller;

			/// @brief Load xml document.
			void UDJAT_API load(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_document &doc);

			/// @brief Verify if the name is a valid factory name.
			const char * validate_name(const char *name);

			/// @brief Register an agent factory.
			void UDJAT_API insert(const Quark &name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method);

			inline void UDJAT_API insert(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {
				insert(Quark(name),method);
			}

			/// @brief Register a node factory.
			void UDJAT_API insert(const Quark &name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> metdho);

			inline void UDJAT_API insert(const char *name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> method) {
				insert(Quark(name),method);
			}

		}

	}


#endif // UDJAT_FACTORY_H_INCLUDED
