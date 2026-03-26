#
# spec file for package libudjat
#
# Copyright (c) <2024> Perry Werneck <perry.werneck@gmail.com>.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://github.com/PerryWerneck/libudjat/issues
#

Summary:		UDJat core library 
Name:			libudjat
Version: 2.3.2
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/libudjat

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

BuildRequires:	binutils
BuildRequires:	coreutils

%if 0%{?suse_version} >= 1504
BuildRequires:	pkgconfig(libeconf)
BuildRequires:	pkgconfig(dmiget)
BuildRequires:	pkgconfig(libsystemd)
BuildRequires:	pkgconfig(vmdetect) >= 1.3
BuildRequires:	pkgconfig(openssl) >= 3.0.0
BuildRequires:	meson >= 0.61.4
%else
BuildRequires:	pkgconfig(libssl)
BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	libiniparser-devel
%endif

%if 0%{?suse_version} >= 1500
BuildRequires:	gcc-c++ 
%else
BuildRequires:	gcc48-c++
%endif

BuildRequires:	pkgconfig(liburiparser)
BuildRequires:	pkgconfig(pugixml)
BuildRequires:	gettext-devel

%description
UDJat core library

Main library for udjat modules.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

%package -n %{name}%{_libvrs}
Summary: UDJat core library
Provides:	%{name}%{MAJOR_VERSION}_%{MINOR_VERSION} = %{version}

%description -n %{name}%{_libvrs}
UDJat core library

Main library for udjat modules.

#---[ Development ]---------------------------------------------------------------------------------------------------

%package devel
Summary:	Development files for %{name}
Requires:	%{name}%{_libvrs} = %{version}

Provides:	libudjat%{MAJOR_VERSION}-devel = %{version}
Provides:	libudjat%{_libvrs}-devel = %{version}
Provides:	udjat-devel = %{version}
Provides:	udjat-rpm-macros = %{version}

%description devel

Development files for Udjat main library.

%lang_package -n %{name}%{_libvrs}

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%autosetup

%if 0%{?suse_version} < 1500
export CC=gcc-4.8
export CXX=g++-4.8
%endif

%if 0%{?suse_version} >= 1504

%meson

%else

# Remove c++ 17 'sugars'
find src -type f -iname *.h -exec sed -i 's@\[\[deprecated.*\]\]@@g' {} \;
find src -type f -iname *.cc -exec sed -i 's@-> .* {@{@g' {} \;

ln -f legacy/* .

export CFLAGS="-Wno-deprecated-declarations %{optflags}"
NOCONFIGURE=1 ./autogen.sh
%configure

%endif

%build
%if 0%{?suse_version} >= 1504
%meson_build
%else
make all
%endif

%install
%if 0%{?suse_version} >= 1504
%meson_install
%else
%makeinstall
%endif

mkdir -p %{buildroot}%{_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}/modules
mkdir -p %{buildroot}%{_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}/lib

%if 0%{?suse_version} >= 1504
%find_lang libudjat-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles
%endif

%files -n %{name}%{_libvrs}
%defattr(-,root,root)
%dir %{_libdir}/udjat
%dir %{_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}
%dir %{_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}/modules
%dir %{_libdir}/udjat/%{MAJOR_VERSION}.%{MINOR_VERSION}/lib
%{_libdir}/%{name}.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%if 0%{?suse_version} >= 1504
%files -n libudjat%{_libvrs}-lang -f langfiles
%endif

%files devel
%defattr(-,root,root)

%if 0%{?suse_version} >= 1504
%{_bindir}/udjat-loader
%endif

%{_libdir}/*.so
%{_libdir}/*.a

%{_libdir}/pkgconfig/*.pc

%if 0%{?suse_version} >= 1504
%{_rpmmacrodir}/macros.*
%else
/usr/lib/rpm/macros.d/macros.*
%endif

%dir %{_includedir}/udjat
%{_includedir}/udjat/*.h

%dir %{_includedir}/udjat/tools
%{_includedir}/udjat/tools/*.h

%dir %{_includedir}/udjat/tools/abstract
%{_includedir}/udjat/tools/abstract/*.h

%dir %{_includedir}/udjat/tools/actions
%{_includedir}/udjat/tools/actions/*.h

%dir %{_includedir}/udjat/tools/file
%{_includedir}/udjat/tools/file/*.h

%dir %{_includedir}/udjat/tools/http
%{_includedir}/udjat/tools/http/*.h

%dir %{_includedir}/udjat/alert
%{_includedir}/udjat/alert/*.h

%dir %{_includedir}/udjat/agent
%{_includedir}/udjat/agent/*.h

%dir %{_includedir}/udjat/linux
%{_includedir}/udjat/linux/*.h

%dir %{_includedir}/udjat/net
%{_includedir}/udjat/net/*

%dir %{_includedir}/udjat/module
%{_includedir}/udjat/module/*

%dir %{_includedir}/udjat/ui
%{_includedir}/udjat/ui/*

%dir %{_includedir}/udjat/tools/url
%{_includedir}/udjat/tools/url/*

%pre -n %{name}%{_libvrs} -p /sbin/ldconfig

%post -n %{name}%{_libvrs} -p /sbin/ldconfig

%postun -n %{name}%{_libvrs} -p /sbin/ldconfig

%changelog

