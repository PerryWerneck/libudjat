

#include "private.h"
#include <cstring>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Request::Request(const char *p) : Json::Value(), path(p) {
	}

	Request::Request(const std::string &p) : Json::Value(), path(p) {
	}


}

