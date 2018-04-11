Name:    NeXus-Streamer
Version: %{_version}
Release: %{_release}%{?dist}
%define dm_group_prefix /opt/dm_group/usr

Summary: NeXus-Streamer
License: BSD 2-clause
URL:     https://github.com/ess-dmsc/NeXus-Streamer
Source:	 NeXus-Streamer-%{version}.tar.gz

BuildArch: x86_64

%description
NeXus-Streamer.


%package -n %{name}-data
Summary: NeXus-Streamer sample data

BuildArch: noarch

%description -n %{name}-data
NeXus-Streamer sample file.


%prep
%setup -q

%install
rm -rf %{buildroot}
install -d %{buildroot}%{dm_group_prefix}/bin
install -d %{buildroot}%{dm_group_prefix}/share/isis_nexus_streamer_for_mantid
cp bin/main_nexusPublisher %{buildroot}%{dm_group_prefix}/bin/nexusPublisher
cp bin/main_nexusSubscriber %{buildroot}%{dm_group_prefix}/bin/nexusSubscriber
cp share/NeXus-Streamer/SANS_test.nxs %{buildroot}%{dm_group_prefix}/share/NeXus-Streamer/
cp share/NeXus-Streamer/LICENSE %{buildroot}%{dm_group_prefix}/share/NeXus-Streamer/

%clean
rm -rf %{buildroot}

%files -n %{name}
%defattr(-,root,root)
%{dm_group_prefix}/bin/nexusPublisher
%{dm_group_prefix}/bin/nexusSubscriber
%doc %{dm_group_prefix}/share/NeXus-Streamer/LICENSE

%files -n %{name}-data
%{dm_group_prefix}/share/NeXus-Streamer/SANS_test.nxs


%changelog

* Wed Feb 01 2017 Matthew D Jones <matthew.d.jones@stfc.ac.uk> 0.1.0
- Initial package
