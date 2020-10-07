/**
 * @file
 *
 * @brief Test event engine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <udjat/event.h>
 #include <udjat/agent.h>
 #include <udjat/service.h>
 #include <iostream>
 #include <pugixml.hpp>
 #include <vector>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	class Event : public Abstract::Event {
	public:
		Event(const pugi::xml_node &node) : Abstract::Event(node) {
		}

	};

	Abstract::Agent agent;
	vector<std::shared_ptr<Event>> events;

	{
		pugi::xml_document doc;
		doc.load_file("test.xml");

		for(auto top : doc.children()) {

			for(pugi::xml_node node = top.child("event"); node; node = node.next_sibling("event")) {
				events.emplace_back(make_shared<Event>(node));
			}

		}

	}

	{
		for(auto event : events) {
			event->set(agent);
		}
	}

	Service::run();

	return 0;
}
