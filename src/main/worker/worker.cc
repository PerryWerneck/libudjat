

#include "private.h"

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Worker::Worker(const Quark &n) : name(n), active(false) {

		static const ModuleInfo info = {

			"",	// The module name.
			"", // The module description.
			"", // The module version.
			"", // The bugreport address.
			"", // The package URL.

		};

		this->info = &info;

		Controller::getInstance().insert(this);
	}

	Worker::~Worker() {
		Controller::getInstance().remove(this);
	}

	void Worker::work(const char *name,const Request &request, Response &response) {

		Worker::Controller::getInstance().find(name)->work(request,response);

		if(response.isNull()) {
			throw system_error(ENODATA,system_category(),"Empty response");
		}

	}


}

