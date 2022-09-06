
#define GNU_SOURCE
#include <config.h>
#include "private.h"
#include <sys/types.h>
#include <dirent.h>
#include <udjat/tools/file.h>
#include <udjat/tools/application.h>
#include <udjat/tools/configuration.h>
#include <udjat/tools/object.h>
#include <udjat/tools/xml.h>

#ifdef _WIN32
	#define MODULE_EXT ".dll"
	#include <udjat/win32/exception.h>
#else
	#include <dlfcn.h>
	#include <unistd.h>
	#define MODULE_EXT ".so"
#endif // _WIN32

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::preload(const char *pathname) {

		// Preload from configuration file.
		{
			Config::Value<vector<string>> modules("modules","load-at-startup","");

			if(!modules.empty()) {
				cout << "modules\tPreloading " << modules.size() << " module(s) from configuration file" << endl;

				Config::Value<bool> required("modules","required",false);
				for(string &module : modules) {
					load(module.c_str(),required);
				}
			}

		}

		if(pathname && *pathname && Config::Value<bool>("modules","preload-from-xml",true)) {

			// Preload from path.
			cout << "modules\tPreloading from " << pathname << endl;
			Udjat::for_each(pathname, [](const char UDJAT_UNUSED(*filename), const pugi::xml_document &doc){
				for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
					if(node.attribute("preload").as_bool(true)) {
						Module::load(node);
					}
				}
			});

		}

	}

	bool Module::load(const pugi::xml_node &node) {
		return Controller::getInstance().load(node);
	}

	bool Module::load(const char *name, bool required) {
		return Controller::getInstance().load(name,required);
	}

	bool Module::Controller::load(const pugi::xml_node &node) {

		const char * name = node.attribute("name").as_string();

		if(!(name && *name)) {
			throw runtime_error("Required attribute 'name' is missing");
		}

		// Check if the module is already loaded.
		if(find(name)) {
#ifdef DEBUG
			cout << "module\t**** The module '" << name << "' was already loaded" << endl;
#endif // DEBUG
			return true;
		}

		// Open module.
		auto handle = open(name,Object::getAttribute(node,"modules","required",true));

		if(!handle) {
			return false;
		}

		try {

			auto module = init(handle,node);
			module->handle = handle;

		} catch(...) {

			close(handle);
			throw;

		}

		return true;

	}

	bool Module::Controller::load(const char *name, bool required) {

		// Check if the module is already loaded.
		if(find(name)) {
#ifdef DEBUG
			cout << "module\t**** The module '" << name << "' was already loaded" << endl;
#endif // DEBUG
			return true;
		}

		auto handle = open(name,required);

		if(!handle)
			return false;

		try {

			auto module = init(handle);
			module->handle = handle;

		} catch(...) {

			close(handle);
			throw;

		}

		return true;

	}

}

