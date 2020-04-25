# run "spectool -g -R infiniband-radar-daemon.spec" to automatically download 
# source files. spectool is part of the rpmdevtools package

%define use_systemd (0%{?fedora} && 0%{?fedora} >= 18) || (0%{?rhel} && 0%{?rhel} >= 7) || (0%{?suse_version} && 0%{?suse_version} >=1210)

Name:		infiniband-radar-daemon
Version:	2.07
Release:	1%{?dist}
Summary:	InfiniBand Radar - Service Daemon

Group:		System Environment/Base
License:	GPLv3
URL:		https://github.com/infiniband-radar/%{name}
Source0:	https://github.com/infiniband-radar/%{name}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:	cmake3 gcc-c++ libibverbs-devel libibmad-devel infiniband-diags-devel libcurl-devel
%{?systemd_requires}
BuildRequires: systemd
Requires:	libibverbs libibmad

%description
%{name} is a tool to report metrics and topologies of Infiniband fabrics.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake3 .
%make_build

%install
%{__mkdir} -p %{buildroot}%{_sysconfdir}/infiniband-radar/
%{__mkdir} -p %{buildroot}%{_sbindir}
%{__install} -m755 infiniband_radar_daemon \
    %{buildroot}%{_sbindir}/infiniband_radar_daemon
%{__install} -m644 config.template.json \
    %{buildroot}%{_sysconfdir}/infiniband-radar/config.template.json
%if %{use_systemd}
    mkdir -p %{buildroot}%{_unitdir}
    %{__install} -m644 infiniband-radar@.service \
        %{buildroot}%{_unitdir}/infiniband-radar@.service
%endif

%post
%if %{use_systemd}
    %systemd_post infiniband-radar@.service
%endif

%preun
%if %{use_systemd}
    %systemd_preun infiniband-radar@.service
%endif

%postun
%if %{use_systemd}
    %systemd_postun infiniband-radar@.service
%endif

%files
%{_sbindir}/infiniband_radar_daemon
%{_sysconfdir}/infiniband-radar/config.template.json
%{_unitdir}/infiniband-radar@.service


%changelog
* Fri Apr 24 2020 Kilian Cavalotti <kilian@stanford.edu>
- initial package
