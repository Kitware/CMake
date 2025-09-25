# This spec file is used to check if the provided version of rpmbuild supports the "Supplements:" tag

Name:           test
Version:        0
Release:        1
Summary:        test
License:        test

Supplements: supplemented_package

%description


%prep

%build
%configure
%install
%clean
%files
%doc
%changelog
