%define version 0.3.2
%define release 1mdk

%define libname_orig %{name}
%define libname %mklibname chewing 0

Name:		libchewing
Summary:	The intelligent Chinese phonetic input method library
Version:	%{version}
Release:	%{release}
Group:		System/Internationalization
License:	LGPL
Source0:	%{name}-%{version}.tar.bz2
URL:		http://chewing.csie.net/
Buildroot:      %_tmppath/%{name}-%{version}-%{release}-root
Requires:	glibc
BuildRequires:	glibc-devel

%description
This is an intelligent Chinese phonetic input method library, which abstracts
the behavior of popular Taiwanese/Chinese intelligent phonetic input method
usage into several APIs ready for integration.  The known examples are
xcin-chewing, scim-chewing, iiimf-le-chewing, etc.

%package data
Summary:	Data for Libchewing.
Group:		System/Internationalization
Requires:	%{libname} = %{version}-%{release}
Provides:       %{libname_orig}-data = %{version}-%{release}

%description data
Data for Libchewing.

%package -n	%{libname}
Summary:	Libchewing library.
Group:		System/Internationalization
Requires:       %{libname_orig}-data = %{version}-%{release}
Provides:	%{libname_orig} = %{version}-%{release}

%description -n %{libname}
Libchewing library.

%package -n	%{libname}-devel
Summary:	Headers of libchewing for development.
Group:		Development/C
Requires:	%{libname} = %{version}-%{release}
Provides:	%{name}-devel = %{version}-%{release}
Provides:	%{libname_orig}-devel = %{version}-%{release}

%description -n %{libname}-devel
Headers of %{name} for development.


%prep
%setup -q

%build

%configure2_5x 
%make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall_std

%clean
rm -rf $RPM_BUILD_ROOT

%post -n %{libname} -p /sbin/ldconfig
%postun -n %{libname} -p /sbin/ldconfig


%files data
%defattr(-,root,root)
%doc COPYING
%{_datadir}/chewing/*

%files -n %{libname}
%defattr(-,root,root)
%doc COPYING
%{_libdir}/lib*.so.1.0.0

%files -n %{libname}-devel
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog NEWS README
%{_libdir}/lib*.so
%{_libdir}/lib*.so.1
%{_libdir}/*.a
%{_libdir}/*.la
%{_libdir}/pkgconfig/*
%{_includedir}/*


%changelog
* Tue Dec 28 2004 Shiva Huang <blueshiva@giga.net.tw> 0.2.5-1shiva
- new version 0.2.5

* Fri Dec 17 2004 Shiva Huang <blueshiva@giga.net.tw> 0.2.4-4shiva
- Fix Requires for libchewing0.
- change URL

* Tue Dec 14 2004 UTUMI Hirosi <utuhiro78@yahoo.co.jp> 0.2.4-3ut
- rewrite spec
- modify the privilege of libchewing.spec
- bzme libchewing-0.2.4.tgz
- change URL
- change Group

* Wed Dec 01 2004 Shiva Huang <blueshiva@giga.net.tw> 0.2.4-2mdk.shiva
- Fixed libraries doesn't have the "so" (libfoo.0.0.0 instead of 
  libfoo.so.0.0.0) problem by official way mentioned by Mandrake
  RpmHowTo.
- Sign this package.

* Sat Oct 09 2004 Shiva Huang <blueshiva@giga.net.tw> 0.2.4-1mdk.shiva
- Update to 0.2.4

* Mon Aug 09 2004 Shiva Huang <blueshiva@giga.net.tw> 0.2.3-1mdk.shiva
- Update to 0.2.3

* Sat Aug 07 2004 Shiva Huang <blueshiva@giga.net.tw> 0.2.2-2mdk
- Fixed BuildRequires and Requires tags

* Fri Aug 06 2004 Shiva Huang <blueshiva@giga.net.tw> 0.2.2-1mdk
- Initial spec
