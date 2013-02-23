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

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc README CONTRIBUTIONS.TXT
%{_bindir}/*
%{_datadir}/btbuilder

