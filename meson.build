# Maintainer: Perry Werneck <perry.werneck@gmail.com>

# SPDX-License-Identifier: LGPL-3.0-or-later 

# Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

project(
	'udjat', 
	['cpp'],
	version: '1.2.0',
	default_options : ['c_std=c11', 'cpp_std=c++17'],
	license: 'GPL-3.0-or-later',
)

module_build=20240725
project_description = 'UDJat core library'
product_name = 'udjat'
product_domain = 'br.eti.werneck'
product_title = 'The eye of horus'

lib_src = []

#
# Versioning
#
pkg_version = meson.project_version()
version_array = pkg_version.split('.')
pkg_major_version = version_array[0].to_int()
pkg_minor_version = version_array[1].to_int()
pkg_micro_version = version_array[2].to_int()

libversion = '@0@.@1@'.format(pkg_major_version,pkg_minor_version)

#
# i18n
#
subdir('po')

#
# Dependencies
#
lib_deps = [
  dependency('libeconf'),
  dependency('libsystemd'),
  dependency('pugixml'),  
]

#
# Compiler flags
#
cxx = meson.get_compiler('cpp')
enable_debug = get_option('debug') or get_option('buildtype').contains('debug')
core_build = run_command('date','+%Y%m%d').stdout().strip()

compiler_flags = [
  '-ggdb3',
  '-ffat-lto-objects',
  '-fvisibility=hidden',
  '-pthread',
  '-fPIC',
  '-DHAVE_CONFIG_H=1',
  '-Wno-overloaded-virtual',
  '-DBUILD_DATE='+core_build,
]

if enable_debug
  compiler_flags += [
	  '-DDEBUG=1'
  ]
else
  compiler_flags += [
	'-DNDEBUG=1'
  ]
endif

add_project_arguments(cxx.get_supported_arguments(compiler_flags), language: 'cpp')

#
# Configuration
#
app_conf = configuration_data()
app_conf.set_quoted('PACKAGE_NAME', meson.project_name())
app_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
app_conf.set_quoted('PACKAGE_DESCRIPTION', project_description)
app_conf.set('PACKAGE_VERSION_MAJOR', pkg_major_version)
app_conf.set('PACKAGE_VERSION_MINOR', pkg_minor_version)
app_conf.set('PACKAGE_VERSION_MICRO', pkg_micro_version)
app_conf.set('MINIMAL_MODULE_BUILD', module_build)

app_conf.set('HAVE_STRPTIME', cxx.has_function('strptime'))

app_conf.set_quoted('PRODUCT_TITLE', product_title)
app_conf.set('PRODUCT_NAME',product_name)
app_conf.set('PRODUCT_DOMAIN',product_domain)
app_conf.set_quoted('PRODUCT_TITLE',product_title)

if cxx.compiles('#include <unistd.h>', name : 'unistd.h')
  app_conf.set('HAVE_UNISTD_H', 1)
endif

#
# SDK
#
sdk_conf = configuration_data()
sdk_conf.set('PACKAGE_NAME', 'udjathttp')
sdk_conf.set('PACKAGE_VERSION', meson.project_version())
sdk_conf.set('PACKAGE_VERSION_MAJOR', pkg_major_version)
sdk_conf.set('PACKAGE_VERSION_MINOR', pkg_minor_version)
sdk_conf.set('PACKAGE_VERSION_MICRO', pkg_micro_version)
sdk_conf.set('PACKAGE_DESCRIPTION','HTTP module and library for udjat')
sdk_conf.set('PACKAGE_CORE_BUILD',core_build)
sdk_conf.set('MODULE_ENGINE_BUILD',module_build)
sdk_conf.set('prefix',get_option('prefix'))
sdk_conf.set('exec_prefix',get_option('prefix'))
sdk_conf.set('includedir',get_option('includedir'))
sdk_conf.set('libdir',get_option('libdir'))

sdk_conf.set('PRODUCT_TITLE', product_title)
sdk_conf.set('PRODUCT_NAME',product_name)
sdk_conf.set('PRODUCT_DOMAIN',product_domain)
sdk_conf.set_quoted('PRODUCT_TITLE',product_title)

includes_dir = include_directories('../src/include')

#
# Sources
#
lib_src += [
  'src/library/agent/agent.cc',
  'src/library/agent/child.cc',
  'src/library/agent/controller.cc',
  'src/library/agent/get.cc',
  'src/library/agent/path.cc',
  'src/library/agent/properties.cc',
  'src/library/agent/set.cc',
  'src/library/agent/setup/alerts.cc',
  'src/library/agent/setup/properties.cc',
  'src/library/agent/start.cc',
  'src/library/agent/state.cc',
  'src/library/agent/update.cc',
  'src/library/agent/factory.cc',
  'src/library/agent/root.cc',
  'src/library/agent/setup.cc',
  'src/library/alert/abstract.cc',
  'src/library/alert/activation.cc',
  'src/library/alert/controller.cc',
  'src/library/alert/factory.cc',
  'src/library/alert/file.cc',
  'src/library/alert/script.cc',
  'src/library/alert/url.cc',
  'src/library/factory/controller.cc',
  'src/library/factory/factory.cc',
  'src/library/load/updater.cc',
  'src/library/module/controller.cc',
  'src/library/module/info.cc',
  'src/library/module/module.cc',
  'src/library/module/unload.cc',
  'src/library/module/load.cc',
  'src/library/request/exec.cc',
  'src/library/request/report.cc',
  'src/library/request/response.cc',
  'src/library/request/responseobject.cc',
  'src/library/request/request.cc',
  'src/library/state/child.cc',
  'src/library/state/level.cc',
  'src/library/state/parse.cc',
  'src/library/state/state.cc',
  'src/library/worker/controller.cc',
  'src/library/worker/worker.cc',
  'src/library/tools/abstract/response.cc',
  'src/library/tools/activatable.cc',
  'src/library/tools/application/application.cc',
  'src/library/tools/application/init.cc',
  'src/library/tools/application/run.cc',
  'src/library/tools/application/setup.cc',
  'src/library/tools/arguments.cc',
  'src/library/tools/base64.cc',
  'src/library/tools/cache.cc',
  'src/library/tools/config.cc',
  'src/library/tools/configuration.cc',
  'src/library/tools/converters.cc',
  'src/library/tools/event.cc',
  'src/library/tools/eventcontroller.cc',
  'src/library/tools/expand.cc',
  'src/library/tools/file/file.cc',
  'src/library/tools/file/watcher.cc',
  'src/library/tools/file/path.cc',
  'src/library/tools/file/handler.cc',
  'src/library/tools/http/error.cc',
  'src/library/tools/http/exception.cc',
  'src/library/tools/http/timestamp.cc',
  'src/library/tools/http/client.cc',
  'src/library/tools/http/mimetype.cc',
  'src/library/tools/ip.cc',
  'src/library/tools/mainloop/service.cc',
  'src/library/tools/mainloop/handler.cc',
  'src/library/tools/mainloop/mainloop.cc',
  'src/library/tools/mainloop/timer.cc',
  'src/library/tools/message.cc',
  'src/library/tools/nic.cc',
  'src/library/tools/quark.cc',
  'src/library/tools/requestpath.cc',
  'src/library/tools/script.cc',
  'src/library/tools/shortcut.cc',
  'src/library/tools/singleton.cc',
  'src/library/tools/string/xml.cc',
  'src/library/tools/string/expand.cc',
  'src/library/tools/string/string.cc',
  'src/library/tools/subprocess.cc',
  'src/library/tools/systemservice.cc',
  'src/library/tools/timestamp.cc',
  'src/library/tools/url/controller.cc',
  'src/library/tools/url/header.cc',
  'src/library/tools/url/protocol.cc',
  'src/library/tools/url/protocols/file.cc',
  'src/library/tools/url/protocols/script.cc',
  'src/library/tools/url/unescape.cc',
  'src/library/tools/url/watcher.cc',
  'src/library/tools/url/url.cc',
  'src/library/tools/url/worker.cc',
  'src/library/tools/value/abstract.cc',
  'src/library/tools/value/csv.cc',
  'src/library/tools/value/html.cc',
  'src/library/tools/value/json.cc',
  'src/library/tools/value/sh.cc',
  'src/library/tools/value/xml.cc',
  'src/library/tools/value/yaml.cc',
  'src/library/tools/xml/document.cc',
  'src/library/tools/xml/attribute.cc',
  'src/library/tools/xml/misc.cc',
  'src/library/tools/exception.cc',
  'src/library/tools/logger.cc',
  'src/library/tools/object.cc',
]

test_src = [
  'src/testprogram/randomfactory.cc',
  'src/testprogram/service.cc',
  'src/testprogram/application.cc',
  'src/testprogram/run.cc',
]

#
# Headers
#
sdk_headers = [
]

#
# OS dependencies
#
if host_machine.system() == 'windows'
  sdk_conf.set('REQUIRES','vmdetect-static pugixml')
  sdk_conf.set('DYNLIBS','-ludjathttp.dll -lws2_32 -lwinhttp')
  sdk_conf.set('STATICLIBS','-ludjathttp -lws2_32 -lwinhttp')
  app_conf.set_quoted('LIBEXT', '.dll')

  lib_deps = [
    dependency('dmiget-static'),
    dependency('vmdetect-static'),
  ]

  install_headers(
    'src/include/udjat/win32/charset.h',
    'src/include/udjat/win32/cleanup.h',
    'src/include/udjat/win32/com.h',
    'src/include/udjat/win32/container.h',
    'src/include/udjat/win32/exception.h',
    'src/include/udjat/win32/handler.h',
    'src/include/udjat/win32/ip.h',
    'src/include/udjat/win32/path.h',
    'src/include/udjat/win32/registry.h',
    'src/include/udjat/win32/security.h',
    'src/include/udjat/win32/service.h',
    subdir: 'udjat/win32'  
  )

  lib_src += [
  ]

else
  sdk_conf.set('REQUIRES','vmdetect-static libeconf pugixml')
  sdk_conf.set('DYNLIBS','-ludjat')
  sdk_conf.set('STATICLIBS','-l:libudjat.a')
  app_conf.set_quoted('LIBEXT', '.so')

  install_headers(
    'src/include/udjat/linux/network.h',
    subdir: 'udjat/linux'  
  )

  lib_src += [
    'src/library/module/os/linux/close.cc',
    'src/library/module/os/linux/init.cc',
    'src/library/module/os/linux/locate.cc',
    'src/library/module/os/linux/tools.cc',
    'src/library/os/linux/icon.cc',
    'src/library/tools/mainloop/os/linux/service.cc',
    'src/library/tools/mainloop/os/linux/handler.cc',
    'src/library/tools/mainloop/os/linux/instance.cc',
    'src/library/tools/mainloop/os/linux/mainloop.cc',
    'src/library/tools/mainloop/os/linux/run.cc',
    'src/library/tools/os/linux/application/cache.cc',
    'src/library/tools/os/linux/application/init.cc',
    'src/library/tools/os/linux/application/application.cc',
    'src/library/tools/os/linux/arguments.cc',
    'src/library/tools/os/linux/event.cc',
    'src/library/tools/os/linux/eventcontroller.cc',
    'src/library/tools/os/linux/file.cc',
    'src/library/tools/os/linux/file/copy.cc',
    'src/library/tools/os/linux/file/list.cc',
    'src/library/tools/os/linux/file/move.cc',
    'src/library/tools/os/linux/file/sysconfig.cc',
    'src/library/tools/os/linux/file/sysconfig_value.cc',
    'src/library/tools/os/linux/file/temporary.cc',
    'src/library/tools/os/linux/file/text.cc',
    'src/library/tools/os/linux/file/path.cc',
    'src/library/tools/os/linux/file/watcher.cc',
    'src/library/tools/os/linux/hostname.cc',
    'src/library/tools/os/linux/ip.cc',
    'src/library/tools/os/linux/nic.cc',
    'src/library/tools/os/linux/protocol.cc',
    'src/library/tools/os/linux/shortcut.cc',
    'src/library/tools/os/linux/subprocess/controller.cc',
    'src/library/tools/os/linux/subprocess/handler.cc',
    'src/library/tools/os/linux/subprocess/init.cc',
    'src/library/tools/os/linux/subprocess/run.cc',
    'src/library/tools/os/linux/subprocess/start.cc',
    'src/library/tools/os/linux/subprocess/subprocess.cc',
    'src/library/tools/os/linux/systemservice.cc',
    'src/library/tools/os/linux/threadpool.cc',
    'src/library/tools/os/linux/econf.cc',
    'src/library/tools/os/linux/logger.cc',
  ]

endif

#
# Targets
#
config_src = [
  configure_file(
      output : 'config.h', 
      configuration : app_conf
  )
]

dynamic = shared_library(
  meson.project_name(), 
  config_src + lib_src,
  install: true,
  version : libversion,
  soversion : libversion,
  gnu_symbol_visibility: 'hidden',
  dependencies: lib_deps,
  include_directories: includes_dir
)

static_library(
  meson.project_name(), 
  config_src + lib_src,
  install: true,
  dependencies: lib_deps,
  include_directories: includes_dir
)

tests = static_library(
  meson.project_name() + 'test', 
  config_src + test_src,
  install: true,
  dependencies: lib_deps,
  include_directories: includes_dir
)

executable(
  'udjat',
  config_src + [ 'src/testprogram/testprogram.cc' ],
  install: false,
  link_with : [ tests, dynamic ],
  dependencies: lib_deps,
  include_directories: includes_dir
)

configure_file(
  input : 'sdk/dynamic.pc.in',
  output : 'udjat.pc',
  install: true,
  install_dir: get_option('libdir') + '/pkgconfig',
  configuration : sdk_conf
)

configure_file(
  input : 'sdk/static.pc.in',
  output : 'udjat-static.pc',
  install: true,
  install_dir: get_option('libdir') + '/pkgconfig',
  configuration : sdk_conf
)

configure_file(
  input : 'src/include/udjat/version.h.in',
  output : 'version.h',
  install: true,
  install_dir: get_option('includedir') + '/udjat',
  configuration : sdk_conf
)

install_headers(
  'src/include/udjat.h',
  'src/include/udjat/worker.h',
  'src/include/udjat/agent.h',
  'src/include/udjat/alert.h',
  'src/include/udjat/defs.h',
  'src/include/udjat/factory.h',
  'src/include/udjat/module.h',
  'src/include/udjat/moduleinfo.h',
  'src/include/udjat/request.h',
  subdir: 'udjat'  
)

install_headers(
  'src/include/udjat/agent/level.h',
  'src/include/udjat/agent/state.h',
  'src/include/udjat/agent/abstract.h',
  subdir: 'udjat/agent'  
)

install_headers(
  'src/include/udjat/alert/abstract.h',
  'src/include/udjat/alert/activation.h',
  'src/include/udjat/alert/file.h',
  'src/include/udjat/alert/proxy.h',
  'src/include/udjat/alert/script.h',
  'src/include/udjat/alert/url.h',
  subdir: 'udjat/alert'  
)

install_headers(
  'src/include/udjat/module/info.h',
  'src/include/udjat/module/abstract.h',
  subdir: 'udjat/module'  
)

install_headers(
  'src/include/udjat/net/interface.h',
  subdir: 'udjat/net'  
)

install_headers(
  'src/include/udjat/net/ip/address.h',
  subdir: 'udjat/net/ip'  
)

install_headers(
  'src/include/udjat/tools/abstract/request-path.h',
  'src/include/udjat/tools/abstract/response.h',
  'src/include/udjat/tools/abstract/object.h',
  subdir: 'udjat/tools/abstract'  
)

install_headers(
  'src/include/udjat/tools/activatable.h',
  'src/include/udjat/tools/base64.h',
  'src/include/udjat/tools/cleanup.h',
  'src/include/udjat/tools/configuration.h',
  'src/include/udjat/tools/container.h',
  'src/include/udjat/tools/converters.h',
  'src/include/udjat/tools/event.h',
  'src/include/udjat/tools/expander.h',
  'src/include/udjat/tools/intl.h',
  'src/include/udjat/tools/message.h',
  'src/include/udjat/tools/method.h',
  'src/include/udjat/tools/network.h',
  'src/include/udjat/tools/object.h',
  'src/include/udjat/tools/parse.h',
  'src/include/udjat/tools/quark.h',
  'src/include/udjat/tools/report.h',
  'src/include/udjat/tools/response.h',
  'src/include/udjat/tools/script.h',
  'src/include/udjat/tools/service.h',
  'src/include/udjat/tools/singleton.h',
  'src/include/udjat/tools/subprocess.h',
  'src/include/udjat/tools/sysconfig.h',
  'src/include/udjat/tools/systemservice.h',
  'src/include/udjat/tools/threadpool.h',
  'src/include/udjat/tools/timestamp.h',
  'src/include/udjat/tools/value.h',
  'src/include/udjat/tools/application.h',
  'src/include/udjat/tools/exception.h',
  'src/include/udjat/tools/factory.h',
  'src/include/udjat/tools/file.h',
  'src/include/udjat/tools/handler.h',
  'src/include/udjat/tools/logger.h',
  'src/include/udjat/tools/mainloop.h',
  'src/include/udjat/tools/protocol.h',
  'src/include/udjat/tools/request.h',
  'src/include/udjat/tools/string.h',
  'src/include/udjat/tools/timer.h',
  'src/include/udjat/tools/url.h',
  'src/include/udjat/tools/xml.h',
  'src/include/udjat/tools/worker.h',
  subdir: 'udjat/tools'  
)

install_headers(
  'src/include/udjat/tools/file/temporary.h',
  'src/include/udjat/tools/file/watcher.h',
  'src/include/udjat/tools/file/handler.h',
  subdir: 'udjat/tools/file'  
)

install_headers(
  'src/include/udjat/tools/http/error.h',
  'src/include/udjat/tools/http/exception.h',
  'src/include/udjat/tools/http/timestamp.h',
  'src/include/udjat/tools/http/client.h',
  'src/include/udjat/tools/http/mimetype.h',
  subdir: 'udjat/tools/http'  
)

install_headers(
  'src/include/udjat/tools/response/object.h',
  'src/include/udjat/tools/response/table.h',
  'src/include/udjat/tools/response/value.h',
  subdir: 'udjat/tools/response'  
)

install_headers(
  'src/include/udjat/ui/icon.h',
  subdir: 'udjat/ui'  
)
