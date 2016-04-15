Summary: Turn based role-playing game builder and engine
Name: btbuilder
Version: 0.5.8
Release: 1%{?dist}
License: GPLv3+
Url: http://www.identicalsoftware.com/btbuilder
Group: Amusements/Games
Source: http://www.identicalsoftware.com/btbuilder/%{name}-%{version}.tgz
BuildRequires: boost-devel
BuildRequires: expat-devel
BuildRequires: libpng-devel
BuildRequires: physfs-devel
BuildRequires: SDL-devel
BuildRequires: SDL_image-devel
BuildRequires: SDL_mixer-devel
BuildRequires: SDL_mng-devel
BuildRequires: SDL_ttf-devel
BuildRequires: desktop-file-utils

%description
Bt Builder is a turn based role-playing game builder and engine in the style
of the old Bard's Tale series. It completely supports the functionality of the
Bard's Tale Construction Set. The eventual goal is to make a game builder that
can implement the three main Bard's Tale games in addition to Construction Set
games.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
make prefix=%{buildroot} install

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
%doc README COPYING
%{_bindir}/*
%{_datadir}/btbuilder
%{_datadir}/icons/hicolor/*/apps/btbuilder.png
%{_datadir}/applications/btbuilder.desktop
%{_datadir}/appdata/btbuilder.appdata.xml

%changelog
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
