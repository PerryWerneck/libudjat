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
	version: '2.0.2',
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
iconv = dependency('iconv')
intl = dependency('intl', required: false)

# URIPARSER
uriparser = dependency(
  'liburiparser', 
  required: true,
)

# VMDETECT (optional)
vmdetect = dependency(
  'vmdetect', 
  method : 'pkg-config',
  required: false,
)

# DMIGET (optional)
dmiget = dependency(
  'dmiget', 
  method : 'pkg-config',
  required: false,
)

# PUGIXML
pugixml = dependency(
  'pugixml',
  method : 'pkg-config',
  required: true,
)


required_libs = [
  pugixml,
  uriparser
]

private_libs = [
  vmdetect,
  dmiget,
]

extra_libs = [
  intl,
  iconv,
  dependency('threads')
] 

if not vmdetect.found()
  vmdetect = dependency(
      'vmdetect',
      required: false,
      fallback: [ 'vmdetect', 'static_library' ]    
  ) 
  extra_libs += [ vmdetect ]
endif

if not dmiget.found()
  dmiget = dependency(
      'dmiget',
      required: false,
      fallback: [ 'dmiget', 'static_library' ]    
  ) 
  extra_libs += [ dmiget ]
endif

#
# Compiler flags
#
cxx = meson.get_compiler('cpp')

time_epoch = run_command('sh', '-c', 'echo "$SOURCE_DATE_EPOCH"', check : true).stdout().strip()
if time_epoch == ''
  time_epoch = run_command('date','+%Y%m%d', check: true).stdout().strip()
endif

add_project_arguments(cxx.get_supported_arguments([
  '-ggdb3',
  '-fvisibility=hidden',
  '-pthread',
  '-fPIC',
  '-DHAVE_CONFIG_H=1',
  '-Wno-overloaded-virtual',
  '-DBUILD_DATE='+time_epoch,
  get_option('buildtype').contains('debug') ? '-DDEBUG=1' : '-DNDEBUG=1',
]), language: 'cpp')

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
app_conf.set('PACKAGE_CORE_BUILD', time_epoch)

if uriparser.found()
  app_conf.set('HAVE_URIPARSER', 1)
  uri_version = uriparser.version().split('.')
  uri_major_version = uri_version[0].to_int()
  uri_minor_version = uri_version[1].to_int()
  uri_micro_version = uri_version[2].to_int()
  app_conf.set('URI_VERSION_MAJOR', uri_major_version)
  app_conf.set('URI_VERSION_MINOR', uri_minor_version)
  app_conf.set('URI_VERSION_MICRO', uri_micro_version)
endif

if host_machine.system() != 'windows'

  app_conf.set('LIBDIR', 
    '@0@/@1@'.format(
      get_option('prefix'),
      get_option('libdir')
    )
  )

  app_conf.set('DATADIR', 
    '@0@/@1@'.format(
      get_option('prefix'),
      get_option('datadir')
    )
  )

  app_conf.set('LOCALEDIR', 
    '@0@/@1@'.format(
      get_option('prefix'),
      get_option('localedir')
    )
  )

endif

app_conf.set('HAVE_STRPTIME', cxx.has_function('strptime'))
app_conf.set('HAVE_STRCASESTR', cxx.has_function('strcasestr'))
app_conf.set('HAVE_LOCALTIME_R', cxx.has_function('localtime_r'))

app_conf.set('HAVE_PUGIXML', 1)

if intl.found()
  app_conf.set('HAVE_LIBINTL', 1)
endif 

app_conf.set('PRODUCT_TITLE', product_title)
app_conf.set('PRODUCT_NAME',product_name)
app_conf.set('PRODUCT_DOMAIN',product_domain)

includes_dir = include_directories('src/include')

if vmdetect.found()
  app_conf.set('HAVE_VMDETECT', 1)
endif

if dmiget.found()
  app_conf.set('HAVE_SMBIOS', 1)
endif

#
# Check for available options for configuration libraries
#
econf = dependency(
  'libeconf', 
  method : 'pkg-config',
  required: false,
)

if econf.found() 

  app_conf.set('HAVE_ECONF', 1)
  if cxx.has_header_symbol('libeconf.h','econf_readConfigWithCallback')
    app_conf.set('ECONF_HAVE_READ_CONFIG_WITH_CALLBACK', 1)
  endif

else 

  econf = dependency(
    'iniparser', 
    required: false, 
    method : 'pkg-config'
  )
  
  if econf.found()
    app_conf.set('HAVE_INIPARSER', 1)
  endif

endif

private_libs += econf

#
# SystemD  (optional)
#
systemd_dep = dependency('libsystemd', required: false  )
if systemd_dep.found()
  app_conf.set('HAVE_SYSTEMD', 1)
  required_libs += systemd_dep
endif 

#
# Sources
#
lib_src += [
  'src/library/agent/agent.cc',
  'src/library/agent/factory.cc',
  'src/library/agent/event.cc',
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
  'src/library/agent/root.cc',
  'src/library/agent/parse.cc',
  'src/library/alert/construct.cc',
  'src/library/alert/factory.cc',
  'src/library/alert/file.cc',
  'src/library/alert/url.cc',
  'src/library/module/controller.cc',
  'src/library/module/info.cc',
  'src/library/module/module.cc',
  'src/library/module/unload.cc',
  'src/library/module/load.cc',
  'src/library/request/report.cc',
  'src/library/request/response.cc',
  'src/library/request/responseobject.cc',
  'src/library/request/request.cc',
  'src/library/state/level.cc',
  'src/library/state/parse.cc',
  'src/library/state/state.cc',
  'src/library/tools/activatable.cc',
  'src/library/tools/application/application.cc',
  'src/library/tools/application/init.cc',
  'src/library/tools/commandlineparser.cc',
  'src/library/tools/application/run.cc',
  'src/library/tools/application/setup.cc',
  'src/library/tools/application/loader.cc',
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
  'src/library/tools/mainloop/os/' + host_machine.system() + '/timer.cc',
  'src/library/tools/message.cc',
  'src/library/tools/nic.cc',
  'src/library/tools/quark.cc',
  'src/library/tools/requestpath.cc',
  'src/library/tools/script.cc',
  'src/library/tools/shortcut.cc',
  'src/library/tools/string/xml.cc',
  'src/library/tools/string/expand.cc',
  'src/library/tools/string/string.cc',
  'src/library/tools/subprocess.cc',
  'src/library/tools/systemservice.cc',
  'src/library/tools/timestamp.cc',
  'src/library/tools/url/methodfactory.cc',
  'src/library/tools/url/handler.cc',
  'src/library/tools/url/controller.cc',
  'src/library/tools/url/header.cc',
  'src/library/tools/url/unescape.cc',
  'src/library/tools/url/watcher.cc',
  'src/library/tools/url/url.cc',
  'src/library/tools/url/worker.cc',
  'src/library/tools/value/construct.cc',
  'src/library/tools/value/set.cc',
  'src/library/tools/value/get.cc',
  'src/library/tools/value/type.cc',
  'src/library/tools/value/csv.cc',
  'src/library/tools/value/html.cc',
  'src/library/tools/value/json.cc',
  'src/library/tools/value/sh.cc',
  'src/library/tools/value/xml.cc',
  'src/library/tools/value/report.cc',
  'src/library/tools/value/yaml.cc',
  'src/library/tools/xml/document.cc',
  'src/library/tools/xml/attribute.cc',
  'src/library/tools/xml/misc.cc',
  'src/library/tools/xml/load.cc',
  'src/library/tools/exception.cc',
  'src/library/tools/logger.cc',
  'src/library/tools/object.cc',
  'src/library/tools/report.cc',
  'src/library/tools/interface.cc',
  'src/library/tools/action.cc',
  'src/library/request/responsevalue.cc',
  'src/library/module/os/' + host_machine.system() + '/close.cc',
  'src/library/module/os/' + host_machine.system() + '/init.cc',
  'src/library/module/os/' + host_machine.system() + '/locate.cc',
  'src/library/module/os/' + host_machine.system() + '/tools.cc',
  'src/library/tools/os/' + host_machine.system() + '/urlconnect.cc',
  'src/library/tools/os/' + host_machine.system() + '/socket.cc',
  'src/library/tools/os/' + host_machine.system() + '/console.cc',
  'src/library/tools/url/handlers/file.cc',
  'src/library/tools/url/handlers/script.cc',
  'src/library/tools/url/handlers/smbios.cc',
  'src/library/ui/progress.cc',
  'src/library/ui/status.cc',
]

#
# SDK
# https://mesonbuild.com/Pkgconfig-module.html
#
pkg = import('pkgconfig')

pkg_variables = [

  'version_major=@0@'.format(pkg_major_version),
  'version_minor=@0@'.format(pkg_minor_version),
  'version_micro=@0@'.format(pkg_micro_version),

  'product_name=@0@'.format(product_name),
  'product_domain=@0@'.format(product_domain),
  'product_title=@0@'.format(product_title),
  'build_id=@0@'.format(time_epoch),

  'package_name=lib@0@@1@_@2@'.format(product_name,pkg_major_version,pkg_minor_version),

  'module_path=${libdir}/@0@/@1@.@2@/modules/'.format(
    product_name,pkg_major_version,pkg_minor_version,
  ),
  'module_version=@0@_@1@'.format(pkg_major_version,pkg_minor_version),

]

if host_machine.system() == 'windows'

  winlibs = [
      '-lintl', 
      '-lshlwapi', 
      '-lws2_32',
      '-lwtsapi32', 
      '-lole32', 
      '-loleaut32', 
      '-luuid', 
      '-liphlpapi', 
      '-lversion', 
      '-lnetshell', 
      '-lpathcch',
  ]

  pkg.generate(
    name: 'lib' + meson.project_name(),
    description: project_description,
    variables: pkg_variables,
    requires: required_libs,
    libraries: [ '-l' + meson.project_name() + '.dll' ],
    requires_private: private_libs,
  )

else

  pkg.generate(
    name: 'lib' + meson.project_name(),
    description: project_description,
    variables: pkg_variables,
    requires: required_libs,
    libraries: [ '-l' + meson.project_name() ],
    requires_private: private_libs,
  )

endif

if host_machine.system() == 'windows'

  # Supplied by win_iconv-devel-static
  app_conf.set('HAVE_ICONV',1)

  app_conf.set_quoted('LIBEXT', '.dll')

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
    'src/library/os/windows/icon.cc',
    'src/library/tools/mainloop/os/windows/eventworker.cc',
    'src/library/tools/mainloop/os/windows/handler.cc',
    'src/library/tools/mainloop/os/windows/handlercontroller.cc',
    'src/library/tools/mainloop/os/windows/service.cc',
    'src/library/tools/mainloop/os/windows/instance.cc',
    'src/library/tools/mainloop/os/windows/mainloop.cc',
    'src/library/tools/mainloop/os/windows/run.cc',
    'src/library/tools/mainloop/os/windows/hwndproc.cc',
    'src/library/tools/os/windows/adapter.cc',
    'src/library/tools/os/windows/application/application.cc',
    'src/library/tools/os/windows/application/description.cc',
    'src/library/tools/os/windows/application/path.cc',
    'src/library/tools/os/windows/application/utils.cc',
    'src/library/tools/os/windows/application/init.cc',
    'src/library/tools/os/windows/application/logdir.cc',
    'src/library/tools/os/windows/arguments.cc',
    'src/library/tools/os/windows/charset.cc',
    'src/library/tools/os/windows/com.cc',
    'src/library/tools/os/windows/configvalue.cc',
    'src/library/tools/os/windows/event.cc',
    'src/library/tools/os/windows/eventcontroller.cc',
    'src/library/tools/os/windows/exception.cc',
    'src/library/tools/os/windows/file.cc',
    'src/library/tools/os/windows/file/copy.cc',
    'src/library/tools/os/windows/file/list.cc',
    'src/library/tools/os/windows/file/temporary.cc',
    'src/library/tools/os/windows/file/text.cc',
    'src/library/tools/os/windows/file/watcher.cc',
    'src/library/tools/os/windows/file/path.cc',
    'src/library/tools/os/windows/hostname.cc',
    'src/library/tools/os/windows/nic.cc',
    'src/library/tools/os/windows/registry.cc',
    'src/library/tools/os/windows/securityidentifier.cc',
    'src/library/tools/os/windows/service/handler.cc',
    'src/library/tools/os/windows/service/manager.cc',
    'src/library/tools/os/windows/service/security.cc',
    'src/library/tools/os/windows/shortcut.cc',
    'src/library/tools/os/windows/strptime.cc',
    'src/library/tools/os/windows/subprocess/handler.cc',
    'src/library/tools/os/windows/subprocess/init.cc',
    'src/library/tools/os/windows/subprocess/run.cc',
    'src/library/tools/os/windows/subprocess/start.cc',
    'src/library/tools/os/windows/subprocess/subprocess.cc',
    'src/library/tools/os/windows/subprocess/watcher.cc',
    'src/library/tools/os/windows/systemservice.cc',
    'src/library/tools/os/windows/threadpool.cc',
    'src/library/tools/os/windows/utils.cc',
    'src/library/tools/os/windows/win32service.cc',
    'src/library/tools/os/windows/ip.cc',
    'src/library/tools/os/windows/logger.cc',
    'src/library/tools/os/windows/system.cc',
  ]

else

  if cxx.compiles('#include <unistd.h>', name : 'unistd.h')
    app_conf.set('HAVE_UNISTD_H', 1)
  endif

  if cxx.compiles('#include <filesystem>', name : 'filesystem')
    app_conf.set('HAVE_FILESYSTEM_H', 1)
  endif

  app_conf.set_quoted('LIBEXT', '.so')

  install_headers(
    'src/include/udjat/linux/network.h',
    subdir: 'udjat/linux'  
  )

  lib_src += [
    'src/library/os/linux/icon.cc',
    'src/library/tools/mainloop/os/linux/service.cc',
    'src/library/tools/mainloop/os/linux/handler.cc',
    'src/library/tools/mainloop/os/linux/instance.cc',
    'src/library/tools/mainloop/os/linux/mainloop.cc',
    'src/library/tools/mainloop/os/linux/run.cc',
    'src/library/tools/mainloop/os/linux/glib.cc',
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
    'src/library/tools/os/linux/configfile.cc',
    'src/library/tools/os/linux/logger.cc',
    'src/library/tools/os/linux/system.cc',
    'src/library/tools/os/linux/netlink_routes.cc',
  ]

endif

#
# Targets
#
config_h = [
  configure_file(
      output : 'config.h', 
      configuration : app_conf
  )
]

if host_machine.system() == 'windows'

  configure_file(
    input : 'src/library/os/windows/app-manifest.xml.in',
    output : 'app-manifest.xml',
    configuration : app_conf
  )

  # https://mesonbuild.com/Windows-module.html
  windows = import('windows')
  resources = windows.compile_resources(
    configure_file(
      input : 'src/library/os/windows/resources.rc.in',
      output : 'resources.rc',
      configuration : app_conf
    )
  )

  dynamic = shared_library(
    meson.project_name(), 
    config_h + lib_src + resources,
    install: true,
    gnu_symbol_visibility: 'hidden',
    dependencies: required_libs + private_libs + extra_libs,
    include_directories: includes_dir,
    link_args: winlibs,
  )

else

  dynamic = shared_library(
    meson.project_name(), 
    config_h + lib_src,
    install: true,
    version : libversion,
    soversion : libversion,
    cpp_args: [ '-ffat-lto-objects' ],
    gnu_symbol_visibility: 'hidden',
    dependencies: required_libs + private_libs + extra_libs,
    include_directories: includes_dir,
    link_args: [
      '-ldl'
    ]
  )

 endif

 static = static_library(
  meson.project_name(), 
  config_h + lib_src,
  install: true,
  cpp_args: [ '-DSTATIC_LIBRARY', '-fno-lto' ],
  dependencies: required_libs + private_libs + extra_libs,
  include_directories: includes_dir
 )

 static_library = declare_dependency(
  link_with : static,
  include_directories : includes_dir,
  variables: pkg_variables,
 ) 

 dynamic_library = declare_dependency(
  link_with : dynamic,
  include_directories : includes_dir,
  variables: pkg_variables,
 ) 

executable(
  meson.project_name(),
  config_h + [ 'src/testprogram/main.cc' ],
  cpp_args: [ '-DTEST_PROGRAM' ],
  install: false,
  link_with : [ dynamic ],
  dependencies: required_libs + private_libs + extra_libs,
  include_directories: includes_dir
)

executable(
  meson.project_name() + '-loader',
  config_h + [ 'src/testprogram/main.cc' ],
  cpp_args: [ '-DSDK_APP' ],
  install: true,
  link_with : [ dynamic ],
  dependencies: required_libs + private_libs + extra_libs,
  include_directories: includes_dir
)

#version_file = configure_file(
#  input : 'src/include/udjat/version.h.in',
#  output : 'version.h',
#  configuration : app_conf
#)

#
# RPM Macros
#  
rpm = find_program('rpm', required: false)
if rpm.found()
 
  macros_conf = configuration_data()
  macros_conf.set('PRODUCT_NAME',product_name)
  macros_conf.set('PRODUCT_VERSION', meson.project_version())
  macros_conf.set('PRODUCT_VERSION_MAJOR', pkg_major_version)
  macros_conf.set('PRODUCT_VERSION_MINOR', pkg_minor_version)
  macros_conf.set('PRODUCT_VERSION_MICRO', pkg_micro_version)

  macros_conf.set('PREFIX',get_option('prefix'))
  macros_conf.set('LIBDIR',get_option('libdir'))

  configure_file(
    input : 'rpm/macros.in',
    output : 'macros.@0@'.format(product_name),
    install: true,
    install_dir: run_command(rpm, '--eval', '%{_rpmmacrodir}', check: true).stdout().strip(), 
    configuration : macros_conf
  )

endif

install_headers(
  'src/include/udjat.h',
  'src/include/udjat/agent.h',
  'src/include/udjat/alert.h',
  'src/include/udjat/defs.h',
  'src/include/udjat/module.h',
  'src/include/udjat/moduleinfo.h',
  'src/include/udjat/loader.h',
  subdir: 'udjat'  
)

install_headers(
  'src/include/udjat/agent/level.h',
  'src/include/udjat/agent/state.h',
  'src/include/udjat/agent/abstract.h',
  'src/include/udjat/agent/percentage.h',
  subdir: 'udjat/agent'  
)

install_headers(
  'src/include/udjat/alert/file.h',
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
  'src/include/udjat/tools/abstract/object.h',
  subdir: 'udjat/tools/abstract'  
)

install_headers(
  'src/include/udjat/tools/url/handler.h',
  subdir: 'udjat/tools/url'  
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
  'src/include/udjat/tools/interface.h',
  'src/include/udjat/tools/network.h',
  'src/include/udjat/tools/object.h',
  'src/include/udjat/tools/parse.h',
  'src/include/udjat/tools/quark.h',
  'src/include/udjat/tools/response.h',
  'src/include/udjat/tools/script.h',
  'src/include/udjat/tools/service.h',
  'src/include/udjat/tools/singleton.h',
  'src/include/udjat/tools/subprocess.h',
  'src/include/udjat/tools/systemservice.h',
  'src/include/udjat/tools/threadpool.h',
  'src/include/udjat/tools/timestamp.h',
  'src/include/udjat/tools/value.h',
  'src/include/udjat/tools/application.h',
  'src/include/udjat/tools/commandlineparser.h',
  'src/include/udjat/tools/exception.h',
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
  'src/include/udjat/tools/system.h',
  'src/include/udjat/tools/interface.h',
  'src/include/udjat/tools/report.h',
  'src/include/udjat/tools/percentage.h',
  'src/include/udjat/tools/socket.h',
  subdir: 'udjat/tools'  
)

install_headers(
  'src/include/udjat/tools/file/temporary.h',
  'src/include/udjat/tools/file/watcher.h',
  'src/include/udjat/tools/file/handler.h',
  'src/include/udjat/tools/file/path.h',
  'src/include/udjat/tools/file/text.h',
  'src/include/udjat/tools/file/list.h',
  subdir: 'udjat/tools/file'  
)

install_headers(
  'src/include/udjat/tools/http/error.h',
  'src/include/udjat/tools/http/exception.h',
  'src/include/udjat/tools/http/method.h',
  'src/include/udjat/tools/http/timestamp.h',
  'src/include/udjat/tools/http/client.h',
  'src/include/udjat/tools/http/mimetype.h',
  subdir: 'udjat/tools/http'  
)

install_headers(
  'src/include/udjat/ui/icon.h',
  'src/include/udjat/ui/console.h',
  'src/include/udjat/ui/progress.h',
  'src/include/udjat/ui/status.h',
  subdir: 'udjat/ui'  
)

install_headers(
  'src/include/udjat/tools/actions/abstract.h',
  subdir: 'udjat/tools/actions'  
)
