#
# spec file for package libudjat
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

Summary:		UDJat core library 
Name:			libudjat
Version:		1.0
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/udjat

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++

BuildRequires:  pkgconfig(libeconf)
BuildRequires:  pkgconfig(pugixml)
BuildRequires:  pkgconfig(jsoncpp)

%description
UDJat core library

Main library for udjat modules.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

%package -n %{name}%{_libvrs}
Summary:	UDJat core library

%description -n %{name}%{_libvrs}
UDJat core library

Main library for udjat modules.

#---[ Development ]---------------------------------------------------------------------------------------------------

%package -n udjat-devel
Summary: Development files for %{name}
Requires: %{name}%{_libvrs} = %{version}

%description -n udjat-devel

Development files for Udjat main library.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup

NOCONFIGURE=1 \
	./autogen.sh

%configure --disable-static

%build
make all

%install
%makeinstall

%files -n %{name}%{_libvrs}
%defattr(-,root,root)
%{_libdir}/%{name}.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%dir %{_sysconfdir}/udjat.conf.d
%config %{_sysconfdir}/udjat.conf.d/*.conf

%files -n udjat-devel
%defattr(-,root,root)
%dir %{_includedir}/udjat
%dir %{_includedir}/udjat/tools
%{_includedir}/udjat.h
%{_includedir}/udjat/*.h
%{_includedir}/udjat/tools/*.h
%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/*.pc

%pre -n %{name}%{_libvrs} -p /sbin/ldconfig

%post -n %{name}%{_libvrs} -p /sbin/ldconfig

%postun -n %{name}%{_libvrs} -p /sbin/ldconfig

%changelog

