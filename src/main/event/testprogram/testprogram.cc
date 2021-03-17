/**
 * @file
 *
 * @brief Test event engine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <udjat/event.h>
 #include <udjat/agent.h>
 #include <udjat/service.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <pugixml.hpp>
 #include <vector>
 #include <civetweb.h>
 #include <sstream>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

 class Event : public Abstract::Event {
 public:
 	Event(const char *name) : Abstract::Event(name) {
 	}

	Event(const pugi::xml_node &node) : Abstract::Event(node) {
	}

	void emit() override {
		cout << "---> " << TimeStamp() << " Event " << *this << endl;
	}


 };

 static vector<shared_ptr<Event>> events;
 static shared_ptr<Agent<uint32_t>> agent;

#ifdef HAVE_CIVETWEB
static int civet_log(const struct mg_connection *conn, const char *message) {
	clog << message << endl;
	return 1;
}

static int civet_handler(struct mg_connection *conn, void UDJAT_UNUSED(*cbdata)) {

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if(strcasecmp(ri->request_method,"get")) {
		mg_send_http_error(conn, 405, "Invalid request method");
		return 405;
	}

	stringstream response;
	response << "<html><body><pre>";

	try {

		const char * cmd = ri->local_uri + 6;

		cout << "\"" << cmd << "\"" << endl;

		if(!strncasecmp(cmd,"/start/",7)) {
			int id = (atoi(cmd+7))-1;
			if(id < 0 || id > (int) events.size())
				throw runtime_error("Invalid event id");

			events[id]->set(agent.get());

			response << "Starting event " << id << endl;

		} else if(!strncasecmp(cmd,"/set/",5)) {

			agent->set(atoi(cmd+5));
			response << "Agent value set to " << agent->get() << " (" << atoi(cmd+5) << ")" << endl;

		}


	} catch(const exception &e) {

		mg_send_http_error(conn, 500, e.what());
		return 500;

	}

	response << "</pre></body></html>";

	auto str = response.str();
	mg_send_http_ok(conn, "text/html; charset=utf-8", str.size());
	mg_write(conn, str.c_str(), str.size());

	return 200;

}
#endif // HAVE_CIVETWEB

int main(int argc, char **argv) {

	Logger::redirect();

	agent = make_shared<Agent<uint32_t>>(0);

	{
		pugi::xml_document doc;
		doc.load_file("test.xml");

		for(auto top : doc.children()) {

			for(pugi::xml_node node = top.child("event"); node; node = node.next_sibling("event")) {
				events.push_back(make_shared<Event>(node));
			}

		}

		agent->push_back(new Event("test-on-agent"));

	}

	/*
	{
		for(auto event : events) {
			cout << endl << "Starting event " << event << endl;
			event->set(agent);
			cout << "Event " << event << "started" << endl;
		}
	}
	*/

#ifdef HAVE_CIVETWEB
	// https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md
	static const char *port = "8989";
	static const char *options[] = {
		"listening_ports", 			port,
		"request_timeout_ms",		"10000",
		"error_log_file",			"error.log",
		"enable_auth_domain_check",	"no",
		NULL
	};

	mg_init_library(0);

	struct mg_callbacks callbacks;
	memset(&callbacks,0,sizeof(callbacks));
	callbacks.log_message = civet_log;
	struct mg_context *ctx = mg_start(&callbacks, 0, options);

	if (ctx == NULL) {
		throw runtime_error("Cannot start CivetWeb - mg_start failed.");
	}
	mg_set_request_handler(ctx, "/udjat/", civet_handler, 0);

	cout	<< endl << "http://127.0.0.1:" << port << "/udjat/start/1"
			<< endl << "http://127.0.0.1:" << port << "/udjat/set/value"
			<< endl;

#endif // HAVE_CIVETWEB

	Service::run();

#ifdef HAVE_CIVETWEB
	mg_stop(ctx);
#endif // HAVE_CIVETWEB

	events.clear();
	agent.reset();

	return 0;
}
