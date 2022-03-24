

#include "private.h"
#include <udjat/tools/string.h>

#ifndef _WIN32
	#include <dlfcn.h>
#endif // _WIN32

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const char *n, const ModuleInfo &i) : name(n),handle(nullptr),info(i) {
		if(!name) {
			throw system_error(EINVAL,system_category(),"Module name cant be null");
		}
		Controller::getInstance().insert(this);
	}

	Module::~Module() {
#ifdef DEBUG
		cout << name << "\t" <<  __FILE__ << "(" << __LINE__ << ")" << endl;
#endif //
		Controller::getInstance().remove(this);
	}

	void Module::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	const Module * Module::find(const char *name) noexcept {
		return Controller::getInstance().find(name);
	}

	std::string Module::filename() const {
#ifdef _WIN32
		TCHAR path[MAX_PATH];
		if(GetModuleFileName(this->handle, path, MAX_PATH) ) {
			return (const char *) path;
		}
#else
		Dl_info info;
		memset(&info,0,sizeof(info));
		if(dladdr(&this->info, &info) != 0 && info.dli_fname && info.dli_fname[0]) {
			return info.dli_fname;
		}
#endif // _WIN32
		return name;
	}

	void Module::options(const pugi::xml_node &node, std::function<void(const char *name, const char *value)> call) {

		for(pugi::xml_node child = node.child("option"); child; child = child.next_sibling("option")) {

			const char *name = child.attribute("name").as_string();
			if(!(name && *name)) {
				cerr << "module\tIgnoring unnamed attribute" << endl;
				continue;
			}

			call(
				name,
				String(child.attribute("value").as_string()).expand(child).c_str()
			);

		}

	}

}

