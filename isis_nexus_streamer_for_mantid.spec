Name:    isis_nexus_streamer_for_mantid
Version: %{_version}
Release: %{_release}%{?dist}
%define dm_group_prefix /opt/dm_group/usr

Summary: ISIS NeXus Streamer For Mantid
License: BSD 2-clause
URL:     https://github.com/ScreamingUdder/isis_nexus_streamer_for_mantid
Source:	 isis_nexus_streamer_for_mantid-%{version}.tar.gz

BuildArch: x86_64

%description
ISIS NeXus Streamer For Mantid.


%package -n %{name}-data
Summary: ISIS NeXus Streamer For Mantid sample data

BuildArch: noarch

%description -n %{name}-data
ISIS NeXus Streamer For Mantid sample file.


%prep
%setup -q

%install
rm -rf %{buildroot}
install -d %{buildroot}%{dm_group_prefix}/bin
install -d %{buildroot}%{dm_group_prefix}/share/isis_nexus_streamer_for_mantid
cp bin/main_nexusPublisher %{buildroot}%{dm_group_prefix}/bin/nexusPublisher
cp bin/main_nexusSubscriber %{buildroot}%{dm_group_prefix}/bin/nexusSubscriber
cp share/isis_nexus_streamer_for_mantid/SANS_test.nxs %{buildroot}%{dm_group_prefix}/share/isis_nexus_streamer_for_mantid/
cp share/isis_nexus_streamer_for_mantid/LICENSE %{buildroot}%{dm_group_prefix}/share/isis_nexus_streamer_for_mantid/

%clean
rm -rf %{buildroot}

%files -n %{name}
%defattr(-,root,root)
%{dm_group_prefix}/bin/nexusPublisher
%{dm_group_prefix}/bin/nexusSubscriber
%doc %{dm_group_prefix}/share/isis_nexus_streamer_for_mantid/LICENSE

%files -n %{name}-data
%{dm_group_prefix}/share/isis_nexus_streamer_for_mantid/SANS_test.nxs


%changelog

* Wed Feb 01 2017 Matthew D Jones <matthew.d.jones@stfc.ac.uk> 0.1.0
- Initial package

