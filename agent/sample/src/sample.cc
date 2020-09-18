/**
 * @file
 *
 * @brief Implements sample agent
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <udjat/agent.h>
 #include <pugixml.hpp>
 #include <cstdlib>

 using pugi::xml_node;

 extern "C" {

	DLL_PUBLIC std::shared_ptr<Udjat::Abstract::Agent> RandomAgentFactory(const Udjat::Abstract::Agent &parent, const pugi::xml_node &node);
	DLL_PUBLIC std::shared_ptr<Udjat::Abstract::Agent> DefaultAgentFactory(const Udjat::Abstract::Agent &parent, const pugi::xml_node &node);

 }

//---[ Implement ]------------------------------------------------------------------------------------------

 std::shared_ptr<Udjat::Abstract::Agent> RandomAgentFactory(const Udjat::Abstract::Agent &parent, const xml_node &node) {

	class RandomAgent : public Udjat::Agent<unsigned int> {
	public:
		RandomAgent(const Udjat::Abstract::Agent &parent, const pugi::xml_node &node) : Agent<unsigned int>(parent,node,0) {
		}

		virtual ~RandomAgent() {
		}

		bool refresh() override {
			return this->set((unsigned int) rand());
		}

	};

	return std::make_shared<RandomAgent>(parent,node);

 }

 std::shared_ptr<Udjat::Abstract::Agent> DefaultAgentFactory(const Udjat::Abstract::Agent &parent, const xml_node &node) {
 	return RandomAgentFactory(parent,node);
 }
