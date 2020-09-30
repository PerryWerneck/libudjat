
#include "private.h"
#include <udjat/configuration.h>
#include <cstring>
#include <civetweb.h>

#ifdef HAVE_CIVETWEB

static bool enabled = true;

static int log_message(const struct mg_connection *conn, const char *message) {
	clog << message << endl;
	return 1;
}

static int WebHandler(struct mg_connection *conn, void *cbdata) {

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if(strcasecmp(ri->request_method,"get")) {
		mg_send_http_error(conn, 404, "Invalid request method");
		return 405;
	}

	const char * uri = (ri->local_uri + 7);

	if(!strncasecmp(uri,"state/",5)) {

		cout << "State: " << (uri+5) << endl;

	} else if(!strncasecmp(uri,"agent",5)) {

		cout << "Agent: " << (uri+5) << endl;

	} else {

		cout << "Unknown: " << uri << endl;

	}

	mg_send_http_error(conn, 404, "Unknown request");
	return 404;
}

void run_civetweb() {

	cout << "Starting civetweb server" << endl;

	static const char *options[] = {
		"listening_ports", "8990",
		"request_timeout_ms",
		"10000",
		"error_log_file",
		"error.log",
		"enable_auth_domain_check",
		"no",
		NULL
	};

	struct mg_callbacks callbacks;
	memset(&callbacks,0,sizeof(callbacks));

	mg_init_library(0);

	struct mg_context *ctx = mg_start(&callbacks, 0, options);

	if (ctx == NULL) {
		throw runtime_error("Cannot start CivetWeb - mg_start failed.");
	}

	// http://127.0.0.1:8990/udjat/state

	mg_set_request_handler(ctx, "/udjat/", WebHandler, 0);

	while(enabled) {
		sleep(1);
	}

	mg_stop(ctx);

}


#endif // HAVE_CIVETWEB

