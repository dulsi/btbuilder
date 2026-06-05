Summary: Turn based role-playing game builder and engine
Name: btbuilder
Version: 0.5.20
Release: %autorelease
License: GPL-3.0-or-later
Url: http://www.identicalsoftware.com/btbuilder
Source: http://www.identicalsoftware.com/btbuilder/%{name}-%{version}.tgz
BuildRequires: gcc
BuildRequires: gcc-c++
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
BuildRequires: make
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
%autochangelog
