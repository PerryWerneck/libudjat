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

compiler_flags = [
  '-ggdb3',
  '-ffat-lto-objects',
  '-fvisibility=hidden',
  '-pthread',
  '-fPIC',
  '-DHAVE_CONFIG_H=1',
  '-Wno-overloaded-virtual',
  '-DBUILD_DATE='+run_command('date','+%Y%m%d').stdout().strip(),
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
app_conf.set('MINIMAL_MODULE_BUILD', 20240725)

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
  'src/testprogram/testprogram.cc'
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

  lib_src += [
  ]

else
  sdk_conf.set('REQUIRES','vmdetect-static libeconf pugixml')
  sdk_conf.set('DYNLIBS','-ludjat')
  sdk_conf.set('STATICLIBS','-l:libudjat.a')
  app_conf.set_quoted('LIBEXT', '.so')

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

executable(
  'udjat',
  config_src + test_src,
  install: false,
  link_with : [ dynamic ],
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

#
# Common targets
#
install_subdir('src/include/udjat', install_dir : get_option('includedir'))
