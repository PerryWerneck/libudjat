

#include "private.h"

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Worker::Worker(const Quark &n) : name(n), active(false) {
		Controller::getInstance().insert(this);
	}

	Worker::~Worker() {
		Controller::getInstance().remove(this);
	}

	void Worker::work(const char *name,const char *path, const Request &request, Response &response) {
		Worker::Controller::getInstance().find(name)->work(path,request,response);
	}


}

