

#include "private.h"

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static ModuleInfo moduleinfo;

	Worker::Worker(const Quark &n, const ModuleInfo *i) : name(n), info(i) {
		Controller::getInstance().insert(this);
	}

	Worker::Worker(const Quark &n) : Worker(n,&moduleinfo) {
	}

	Worker::~Worker() {
		Controller::getInstance().remove(this);
	}

	void Worker::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	void Worker::work(const char *name,Request &request, Response &response) {

		Worker::Controller::getInstance().find(name)->work(request,response);

		if(response.isNull()) {
			throw system_error(ENODATA,system_category(),"Empty response");
		}

	}


}

