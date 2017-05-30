# Defines a constant that is used later by the spec file.
%define shortname jvlite

# Name, brief description, and version 
Summary: JournalViewer Lite - ISIS journal viewer (user version)
Name: %{shortname}
Version: 1.8.4
Release: 1
License: GPL
%define fullname %{name}-%{version}
# norootforbuild

# Define buildroot
BuildRoot: /var/tmp/%{fullname}

# Software group
Group: Productivity/Scientific/Chemistry

# Source tar ball.
Source: %{fullname}.tar.gz

# Location of the project's home page.
URL: http://www.projectaten.net

# Owner of the product.
Vendor: Tristan Youngs

# Packager of the product.
Packager: Tristan Youngs

# Boolean that specifies if you want to automatically determine some dependencies
AutoReq: yes

# Build dependencies
BuildRequires: gcc-c++ libQt5Core-devel libQt5Core5 libQt5OpenGL-devel libQt5OpenGL5 libQt5Widgets5 libQt5Widgets-devel libQt5Svg5 libQt5Network5 libQt5Network-devel libQt5PrintSupport5 libQt5PrintSupport-devel hdf5-devel

# In-depth description.
%description
JournalViewer provides a GUI application to browse, interrogate, and plot data from ISIS journal files, including access to sample environment data contained within RAW and NXS files.

%prep
%setup -q

%build

# Configure and make
./configure --enable-lite --with-build-dir=$RPM_BUILD_ROOT --with-install-dir=/usr --prefix=$RPM_BUILD_ROOT/usr 
make

%install
make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README TODO COPYING ChangeLog
/usr/share/applications/JournalViewerLite.desktop
/usr/share/pixmaps/JournalViewerLite.png
/usr/bin/jvlite

%changelog
* Wed Apr 02 2008 Tristan Youngs <tristan.youngs@stfc.ac.uk> 
- initial version.
