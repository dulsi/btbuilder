Summary: Turn based role-playing game builder and engine
Name: btbuilder
Version: 0.3.16
Release: 1%{?dist}
License: GPLv2+
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
BuildRequires: SDL_ttf-devel

%description
Bt Builder is an open source implementation of the Bard's Tale Construction Set. The eventual goal is to make a game builder that can implement the three main Bard's Tale games in addition to Construction Set games.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
make prefix=%{buildroot} install
desktop-file-install --dir=%{buildroot}/%{_desktopdir} btbuilder.desktop

%post
update-desktop-database &> /dev/null ||:
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%postun
update-desktop-database &> /dev/null ||:
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc README CONTRIBUTIONS.TXT
%{_bindir}/*
%{_datadir}/btbuilder
%{_datadir}/applications/btbuilder.desktop
%{_datadir}/icons/hicolor/*/apps/btbuilder.png

