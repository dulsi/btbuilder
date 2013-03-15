Summary: Turn based role-playing game builder and engine
Name: btbuilder
%define	version	0.3.12
Version: %{version}
Release: 1%{?dist}
License: GPLv2+
Url: http://www.identicalsoftware.com/btbuilder
Group: Amusements/Games
Source: http://www.identicalsoftware.com/btbuilder/btbuilder-%{version}.tgz

%description
Bt Builder is an open source implementation of the Bard's Tale Construction Set. The eventual goal is to make a game builder that can implement the three main Bard's Tale games in addition to Construction Set games.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
make prefix=%{buildroot} install
desktop-file-install --dir=%{buildroot}/%{_datadir}/applications btbuilder.desktop

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
%{_datadir}/icons/hicolor/16x16/btbuilder.png
%{_datadir}/icons/hicolor/32x32/btbuilder.png
%{_datadir}/icons/hicolor/48x48/btbuilder.png

