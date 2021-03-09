

 #include "private.h"
 #include <ctime>
// #include <cstring>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Response::Response(const time_t e, const time_t m) : Json::Value(), expiration(e), modification(m) { }

	Response::Response() : Response(0,0) { }

	void Response::setExpirationTimestamp(const time_t time) {

		if(expiration) {
			expiration = min(expiration,time);
		} else {
			expiration = time;
		}

	}

	void Response::setModificationTimestamp(const time_t time) {

		if(modification)
			modification = min(modification,time);
		else
			modification = time;

	}

}

