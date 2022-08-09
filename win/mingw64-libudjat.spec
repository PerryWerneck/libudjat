#
# spec file for package mingw64-libudjat
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define __strip %{_mingw64_strip}
%define __objdump %{_mingw64_objdump}
%define __find_requires %{_mingw64_findrequires}
%define __find_provides %{_mingw64_findprovides}
%define __os_install_post %{_mingw64_debug_install_post} \
                          %{_mingw64_install_post}

Summary:		UDJat core library for mingw64
Name:			mingw64-libudjat
Version:		1.0
Release:		0
License:		LGPL-3.0
Source:			libudjat-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/udjat

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++

BuildRequires:	mingw64-cross-binutils
BuildRequires:	mingw64-cross-gcc
BuildRequires:	mingw64-cross-gcc-c++
BuildRequires:	mingw64-cross-pkg-config
BuildRequires:	mingw64-filesystem
BuildRequires:	mingw64-gettext-tools

BuildRequires:	mingw64-pugixml-devel
BuildRequires:	mingw64-vmdetect-devel

%description
UDJat core library

Main library for udjat modules.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

%package -n %{name}%{_libvrs}
Summary:	UDJat core library
Requires:	mingw64-libpugixml
Requires:	mingw64-libvmdetect

%description -n %{name}%{_libvrs}
UDJat core library

Main library for udjat modules.

#---[ Development ]---------------------------------------------------------------------------------------------------

%package devel
Summary: Development files for %{name}
Requires:	mingw64-pugixml-devel
Requires:	mingw64-vmdetect-devel
Requires:	%{name}%{_libvrs} = %{version}

Provides:	mingw64-udjat-devel = %{version}

%description devel

Development files for Udjat main library.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup -n libudjat-%{version}

NOCONFIGURE=1 \
	./autogen.sh

%{_mingw64_configure}

%build
make all %{?_smp_mflags}

%{_mingw64_strip} \
	--strip-all \
	.bin/Release/*.dll

%install
make DESTDIR=%{buildroot} install
mkdir -p %{buildroot}%{_mingw64_libdir}/udjat-modules
mkdir -p %{buildroot}%{_mingw64_libdir}/udjat-modules/%{MAJOR_VERSION}.%{MINOR_VERSION}

%files -n %{name}%{_libvrs}
%defattr(-,root,root)
%dir %{_mingw64_libdir}/udjat-modules
%dir %{_mingw64_libdir}/udjat-modules/%{MAJOR_VERSION}.%{MINOR_VERSION}

%{_mingw64_bindir}/*.dll

%exclude /etc/udjat.conf.d 

%files devel
%defattr(-,root,root)

%{_mingw64_libdir}/*.a

%{_mingw64_libdir}/pkgconfig/*.pc

%dir %{_mingw64_includedir}/udjat
%{_mingw64_includedir}/udjat/*.h

%dir %{_mingw64_includedir}/udjat/tools
%{_mingw64_includedir}/udjat/tools/*.h

%dir %{_mingw64_includedir}/udjat/tools/http
%{_mingw64_includedir}/udjat/tools/http/*.h

%dir %{_mingw64_includedir}/udjat/win32
%{_mingw64_includedir}/udjat/win32/*.h

%dir %{_mingw64_includedir}/udjat/agent
%{_mingw64_includedir}/udjat/agent/*.h

%changelog

