Version: 1.1 
Name: tinytrapd
Summary: Small yet flexible SNMP trap logger. 
Release: 1
Group: System Environment/Daemons 
License: BSD 
Source0: %{name}-%{version}.tar.gz
Prefix: %{_prefix}
Provides: %{name} 
Obsoletes: %{name}
Requires:       boost
BuildRequires:  boost-devel

Requires(post): %{_sbindir}/useradd, /sbin/chkconfig
Requires(preun):        /sbin/service, /sbin/chkconfig

BuildRoot: /var/tmp/%{name}-%{version}.%{release}-root

%description
Small yet flexible configurable SNMP trap logger.

%prep
%setup -q 

%build
aclocal; automake; autoconf
%configure --prefix=/usr --sysconfdir=/etc 
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT

make install DESTDIR=%{buildroot}

%post
if [ $1 -eq 1 ]; then
        /sbin/chkconfig --add %{name}
        userid=`id -u %{name} 2>/dev/null`
        if [ x"$userid" = x ]; then
                %{_sbindir}/useradd -c "Tinytraps user" -s /sbin/nologin -r -d / %{name} > /dev/null || :
        fi
fi
%preun
if [ $1 -eq 0 ]; then
        /sbin/service %{name} stop >/dev/null 2>&1 || :
        /sbin/chkconfig --del %{name}
fi


%clean
rm -rf $RPM_BUILD_ROOT

%files 
%defattr(-,root,root)
%{_bindir}/%{name}
%{_bindir}/mib2cfg.pl
%{_initrddir}/%{name}
%dir %{_datadir}/%{name}/
%dir %{_datadir}/%{name}/mibs
%dir %{_sysconfdir}/%{name}/
%config(noreplace) %{_sysconfdir}/%{name}/%{name}.conf
%{_datadir}/%{name}/mibs/if-mib.mib
%{_datadir}/%{name}/mibs/rfc1151-smi.mib
%{_datadir}/%{name}/mibs/rfc1213-mib.mib
%{_initrddir}/%{name}
%{_datadir}/%{name}/README

%changelog
* Tue Sep 25 2012 Alexey Grachev <alxgrh@yandex.ru> 1.1.0 
- initial RPM release

