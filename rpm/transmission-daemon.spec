Name:           transmission-daemon
Version:        4.0.6
Release:        1
Summary:        Transmission BitTorrent daemon with a Sailfish OS control UI
License:        GPL-2.0-or-later AND MIT
URL:            https://github.com/zenevo/sfos-transmission
Source0:        ui/transmission-daemon.pro

# Leave the binaries exactly as built: they are the ones tested on the device.
%global __os_install_post %{nil}
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Network)

# One package now, where it used to be two: the daemon ("transmission") and
# the Sailfish UI ("transmission-daemon-ui"). Installing this replaces both.
Obsoletes:      transmission < 4.0.7
Provides:       transmission = %{version}
Obsoletes:      transmission-daemon-ui < 1.0.1
Provides:       transmission-daemon-ui = 1.0.0

%description
Transmission is a BitTorrent client.

This package contains, in one:

  * transmission-daemon    - the headless daemon (RPC on 127.0.0.1:9091)
  * transmission-remote    - the remote control utility
  * transmission-cli       - the command line client
  * transmission-daemon-ui - a small Sailfish OS app to start, stop and
                             monitor the local daemon
  * the systemd user unit that runs the daemon in the background
  * the libevent 2.1 runtime the daemon needs, so nothing else has to be
    installed

The daemon and the CLI are built natively for aarch64 (see daemon/README.sfos);
the UI is built from source here.

%prep
# The sources are the folders next to this spec's parent: ui/ (built here)
# and daemon/ (the prebuilt daemon, CLI and libevent).

%build
cd ui
qmake
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}
# the daemon, its libraries, the unit and its notes, as they were built
mkdir -p %{buildroot}%{_bindir}
cp -f daemon/transmission-daemon daemon/transmission-remote daemon/transmission-cli %{buildroot}%{_bindir}/
mkdir -p %{buildroot}/usr/lib/systemd/user %{buildroot}%{_docdir}/%{name}
cp -f daemon/transmission-daemon.service %{buildroot}/usr/lib/systemd/user/
cp -f daemon/README.sfos %{buildroot}%{_docdir}/%{name}/
mkdir -p %{buildroot}/usr/lib
cp -f daemon/libevent*.so.7.0.1 %{buildroot}/usr/lib/
for l in libevent-2.1 libevent_core-2.1 libevent_extra-2.1 libevent_pthreads-2.1; do
  ln -sf ${l}.so.7.0.1 %{buildroot}/usr/lib/${l}.so.7
done
# the UI, built above
cd ui
make install INSTALL_ROOT=%{buildroot}
install -D -m 644 data/transmission-daemon-ui.desktop %{buildroot}%{_datadir}/applications/transmission-daemon-ui.desktop
for s in 86 108 128 172 256 512; do
  install -D -m 644 icons/transmission-daemon-$s.png %{buildroot}%{_datadir}/icons/hicolor/${s}x${s}/apps/transmission-daemon-ui.png
done

%post
systemctl --user daemon-reload >/dev/null 2>&1 || true

%postun
systemctl --user daemon-reload >/dev/null 2>&1 || true

%files
%{_bindir}/transmission-daemon
%{_bindir}/transmission-remote
%{_bindir}/transmission-cli
%{_bindir}/transmission-daemon-ui
/usr/lib/systemd/user/transmission-daemon.service
/usr/lib/libevent-2.1.so.7
/usr/lib/libevent-2.1.so.7.0.1
/usr/lib/libevent_core-2.1.so.7
/usr/lib/libevent_core-2.1.so.7.0.1
/usr/lib/libevent_extra-2.1.so.7
/usr/lib/libevent_extra-2.1.so.7.0.1
/usr/lib/libevent_pthreads-2.1.so.7
/usr/lib/libevent_pthreads-2.1.so.7.0.1
%{_datadir}/transmission-daemon-ui
%{_datadir}/applications/transmission-daemon-ui.desktop
%{_datadir}/icons/hicolor/*/apps/transmission-daemon-ui.png
%{_docdir}/%{name}/README.sfos
