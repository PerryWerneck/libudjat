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
#
# Please submit bugfixes or comments via https://github.com/PerryWerneck/libudjat/issues
#
Summary:		UDJat core library for mingw64
Name:			mingw64-libudjat
Version: 5.5.0
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

BuildRequires:	mingw64-filesystem
BuildRequires:	mingw64-cross-binutils
BuildRequires:	mingw64-cross-gcc
BuildRequires:	mingw64-cross-gcc-c++
BuildRequires:	mingw64-cross-pkg-config
BuildRequires:	mingw64-filesystem
BuildRequires:	mingw64-gettext-tools

BuildRequires:	mingw64(pkg:pugixml)
BuildRequires:	mingw64(pkg:vmdetect)
BuildRequires:	mingw64(lib:intl)
BuildRequires:	mingw64-win_iconv-devel-static

BuildRequires:	fdupes

%description
UDJat core library

Main library for udjat modules.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

%package -n %{name}%{_libvrs}
Summary:	UDJat core library
Provides:	mingw64(lib:udjat.dll)
Provides:	mingw64(lib:udjat)

%description -n %{name}%{_libvrs}
UDJat core library

Main library for udjat modules.

#---[ Development ]---------------------------------------------------------------------------------------------------

%lang_package -n %{name}%{_libvrs}

%package devel
Summary: Development files for %{name}
Requires:	mingw64(pkg:pugixml)
Requires:	mingw64(pkg:vmdetect)
Requires:	mingw64(lib:intl)
Requires:	%{name}%{_libvrs} = %{version}

Provides:	mingw64(lib:udjat) = %{version}
Provides:	mingw64(lib::libudjat.a)

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

%install
%_mingw64_make_install

mkdir -p %{buildroot}%{_mingw64_libdir}/udjat
mkdir -p %{buildroot}%{_mingw64_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}
mkdir -p %{buildroot}%{_mingw64_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}/modules

%_mingw64_find_lang libudjat-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles

%fdupes %{buildroot}

%files -n %{name}%{_libvrs}
%defattr(-,root,root)

%dir %{_mingw64_libdir}/udjat
%dir %{_mingw64_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}
%dir %{_mingw64_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}/modules
%{_mingw64_bindir}/*.dll

%exclude %{_mingw64_sysconfdir}/udjat.conf.d 

%files -n %{name}%{_libvrs}-lang -f langfiles

%files devel
%defattr(-,root,root)

%{_mingw64_libdir}/*.a

%{_mingw64_libdir}/pkgconfig/*.pc

%dir %{_mingw64_includedir}/udjat
%{_mingw64_includedir}/udjat/*.h

%dir %{_mingw64_includedir}/udjat/tools
%{_mingw64_includedir}/udjat/tools/*.h

%dir %{_mingw64_includedir}/udjat/alert
%{_mingw64_includedir}/udjat/alert/*.h

%dir %{_mingw64_includedir}/udjat/tools/http
%{_mingw64_includedir}/udjat/tools/http/*.h

%dir %{_mingw64_includedir}/udjat/tools/file
%{_mingw64_includedir}/udjat/tools/file/*.h

%dir %{_mingw64_includedir}/udjat/tools/response
%{_mingw64_includedir}/udjat/tools/response/*.h

%dir %{_mingw64_includedir}/udjat/net
%{_mingw64_includedir}/udjat/net/*.h

%dir %{_mingw64_includedir}/udjat/net/ip
%{_mingw64_includedir}/udjat/net/ip/*.h

%dir %{_mingw64_includedir}/udjat/win32
%{_mingw64_includedir}/udjat/win32/*.h

%dir %{_mingw64_includedir}/udjat/tools/abstract/
%{_mingw64_includedir}/udjat/tools/abstract/*.h

%dir %{_mingw64_includedir}/udjat/agent
%{_mingw64_includedir}/udjat/agent/*.h

%dir %{_mingw64_includedir}/udjat/module
%{_mingw64_includedir}/udjat/module/*.h

%dir %{_mingw64_includedir}/udjat/ui
%{_mingw64_includedir}/udjat/ui/*.h

%changelog

