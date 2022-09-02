

#include "private.h"
#include <udjat/tools/string.h>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const char *n, const ModuleInfo &i) : name(n),handle(nullptr),info(i) {

		if(i.build && i.build < 20220902) {
			throw system_error(EINVAL,system_category(),"Invalid build date on module");
		}

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

	void Module::set(const pugi::xml_document UDJAT_UNUSED(&document)) {
	}

	void Module::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	const Module * Module::find(const char *name) noexcept {
		return Controller::getInstance().find(name);
	}

	void Module::for_each(std::function<void(Module &module)> method) {
		Controller::getInstance().for_each(method);
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

