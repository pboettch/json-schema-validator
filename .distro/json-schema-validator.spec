Name:           json-schema-validator
Summary:        JSON schema validator for JSON for Modern C++
Version:        0.0.0
Release:        %autorelease
License:        MIT
URL:            https://github.com/pboettch/json-schema-validator

Source:         https://github.com/pboettch/json-schema-validator/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  ninja-build
BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  json-devel

%description
Json schema validator library for C++ projects using nlohmann/json

%package        devel
Summary:        Development files for JSON schema validator
Requires:       json-schema-validator%{?_isa} = %{version}-%{release}
Requires:       json-devel

%description    devel
Json schema validator development files for C++ projects using nlohmann/json


%prep
%autosetup -n json-schema-validator-%{version}


%build
%cmake \
    -DJSON_VALIDATOR_SHARED_LIBS=ON \
    -DJSON_VALIDATOR_INSTALL=ON \
    -DJSON_VALIDATOR_BUILD_EXAMPLES=OFF \
    -DJSON_VALIDATOR_BUILD_TESTS=ON

%cmake_build


%install
%cmake_install


%check
%ctest


%files
%doc README.md
%license LICENSE
%{_libdir}/libnlohmann_json_validator.so.*

%files devel
%{_libdir}/libnlohmann_json_validator.so
%{_includedir}/nlohmann/json-schema.hpp
%{_libdir}/cmake/nlohmann_json_schema_validator


%changelog
%autochangelog
