# This spec file is used to check if the provided version of rpmbuild supports the "Recommends:" tag

Name:           test
Version:        0
Release:        1
Summary:        test
License:        test

Recommends: recommended_package

%description


%prep

%build
%configure
%install
%clean
%files
%doc
%changelog
