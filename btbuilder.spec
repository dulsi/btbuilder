Summary: Turn based role-playing game builder and engine
Name: btbuilder
Version: 0.5.14
Release: 1%{?dist}
License: GPLv3+
Url: http://www.identicalsoftware.com/btbuilder
Group: Amusements/Games
Source: http://www.identicalsoftware.com/btbuilder/%{name}-%{version}.tgz
BuildRequires: boost-devel
BuildRequires: expat-devel
BuildRequires: libpng-devel
BuildRequires: physfs-devel
BuildRequires: SDL2-devel
BuildRequires: SDL2_image-devel
BuildRequires: SDL2_mixer-devel
BuildRequires: SDL_mng-devel
BuildRequires: SDL2_ttf-devel
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib
Requires:       %{name}-data = %{version}
Requires: hicolor-icon-theme

%description
Bt Builder is a turn based role-playing game builder and engine in the style
of the old Bard's Tale series. It completely supports the functionality of the
Bard's Tale Construction Set. The eventual goal is to make a game builder that
can implement the three main Bard's Tale games in addition to Construction Set
games.

%package	data
Summary:	%{summary}
Requires:	%{name} = %{version}
BuildArch:      noarch

%description	data
This package contains the data files for Bt Builder.

%prep
%setup -q

%build
%make_build CFLAGS="%{optflags}"

%install
make prefix=%{buildroot} install

%check
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/appdata/*.appdata.xml

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%files
%doc README
%{_bindir}/*
%{_datadir}/icons/hicolor/*/apps/btbuilder.png
%{_datadir}/applications/btbuilder.desktop
%{_datadir}/appdata/btbuilder.appdata.xml

%files data
%license COPYING
%{_datadir}/btbuilder

%changelog
* Wed Nov 11 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.14-1
- New version of btbuilder released.

* Wed Oct 19 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.13-1
- New version of btbuilder released.

* Wed Sep 21 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.12-1
- New version of btbuilder released.
- Add validation of appdata file.
- Require hicolor icons.

* Fri Sep 02 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.11-2
- Separate data files into seperate package.

* Sat Aug 20 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.11-1
- New version of btbuilder released.

* Tue Jul 12 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.9-1
- New version of btbuilder released.

* Thu Apr 14 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.8-1
- New version of btbuilder released.

* Wed Jan 13 2016 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.7-1
- New version of btbuilder released.

* Thu Dec 24 2015 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.6-1
- New version of btbuilder released.
- appdata file added.

* Fri Aug 07 2015 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.5-1
- New version of btbuilder released.

* Mon Jul 20 2015 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.4-1
- New version of btbuilder released.

* Sun May 31 2015 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.3-1
- New version of btbuilder released.

* Fri May 1 2015 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.2-1
- New version of btbuilder released.

* Thu Apr 16 2015 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.1-1
- New version of btbuilder released.

* Mon Sep 29 2014 Dennis Payne <dulsi@identicalsoftware.com> - 0.5.0-1
- New version of btbuilder released.

* Mon Jul 21 2014 Dennis Payne <dulsi@identicalsoftware.com> - 0.4.7-1
- New version of btbuilder released.

* Tue Jun 17 2014 Dennis Payne <dulsi@identicalsoftware.com> - 0.4.6-1
- New version of btbuilder released.

* Fri May 9 2014 Dennis Payne <dulsi@identicalsoftware.com> - 0.4.5-1
- New version of btbuilder released.

* Mon Apr 14 2014 Dennis Payne <dulsi@identicalsoftware.com> - 0.4.4-1
- New version of btbuilder released.

* Mon Mar 24 2014 Dennis Payne <dulsi@identicalsoftware.com> - 0.4.3-1
- Modifications based on review of spec file for Fedora.
- New version of btbuilder released.
