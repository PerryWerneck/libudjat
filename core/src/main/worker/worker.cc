

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

}

