# Transmission daemon + Sailfish OS UI

Transmission **4.0.6** for Sailfish OS (aarch64), packaged as **one RPM**: the
headless daemon, its CLI tools, the runtime it needs, the systemd user unit and
a small Sailfish app to control it.

Originally two packages (`transmission` and `transmission-daemon-ui`); they are
now merged. The merged package `Obsoletes` both, so updating replaces them.

![Transmission Daemon app](ui/icons/transmission-daemon-256.png)

Chinese documentation: [README.zh-CN.md](README.zh-CN.md)

---

## What is in the package

| Path | What |
|---|---|
| `/usr/bin/transmission-daemon` | the headless BitTorrent daemon (RPC on `127.0.0.1:9091`) |
| `/usr/bin/transmission-remote` | remote control utility |
| `/usr/bin/transmission-cli` | command line client |
| `/usr/bin/transmission-daemon-ui` | the Sailfish app (start/stop switch + status) |
| `/usr/lib/libevent-2.1.so.7*` | libevent 2.1 runtime, bundled (Sailfish OS has none) |
| `/usr/lib/systemd/user/transmission-daemon.service` | runs the daemon as a user service |
| `/usr/share/transmission-daemon-ui/qml/` | the app's QML |
| `/usr/share/applications/transmission-daemon-ui.desktop` | launcher entry |

## Install

```sh
rpm -Uvh --nodeps --replacepkgs --ignorearch transmission-daemon-4.0.6-1.aarch64.rpm

systemctl --user daemon-reload
systemctl --user enable --now transmission-daemon
```

The package is available from the [Releases](../../releases) page.

## Use

- RPC: `127.0.0.1:9091`, no authentication;
- download directory: `~/Downloads`;
- any Transmission RPC client works (Tremotesf, transmission-remote, …);
- the app is **Transmission Daemon** in the launcher.

## Why this build

1. **Not 3.00.** Transmission 3.00 aborts on this device (OpenSSL 3.5) as soon as
   it hashes a torrent:
   `assertion failed: handle != NULL (crypto-utils-openssl.c:210)` → SIGABRT.
   4.0.6 supports OpenSSL 3 natively.
2. **Built natively.** The Sailfish SDK cross environment could not satisfy
   transmission's CMake (`find_package(CURL)` resolution), so the daemon was
   built **on the phone** (gcc 13.4 + cmake 3.31 from the Jolla repository).
   The binaries in `daemon/` are exactly what that build produced.
3. **libevent bundled.** The Sailfish OS repositories have no libevent, so the
   `libevent-2.1.so.7*` built alongside transmission ships inside this package.
4. **miniupnpc populated.** The upstream tarball's `third-party/miniupnpc` is an
   empty directory (a git submodule); it was filled from
   `third-party/miniupnp/miniupnpc` before building.
5. **Why a UI.** The daemon is a background service; the app gives a simple
   start/stop switch and a status view. It controls the user service with
   `systemctl --user start/stop transmission-daemon` and reads version, torrent
   count and transfer speeds from the RPC at `127.0.0.1:9091`.
6. **The UI runs without the sandbox.** Sailfish 5.2 puts apps into the sailjail
   sandbox, which blocks `systemctl --user`, so the switch could not start or
   stop the service. The `.desktop` file therefore carries
   `[X-Sailjail] Sandboxing=Disabled`. This is a local system utility and the
   trade-off is deliberate; stopping also works through the RPC `session-close`
   without any sandbox permission.

## Building

The UI is built from source; the daemon binaries are shipped prebuilt.

```sh
./build.sh          # runs mb2 inside the Sailfish SDK chroot
```

The spec (`rpm/transmission-daemon.spec`) compiles `ui/`, installs the prebuilt
files from `daemon/`, and produces
`RPMS/transmission-daemon-4.0.6-1.aarch64.rpm`.

To rebuild the daemon itself, build transmission 4.0.6 on the device as
described in `daemon/README.sfos` and drop the resulting binaries into
`daemon/`.

## Notes

- On restricted networks **P2P/trackers need a VPN**; without one torrents stay
  `Idle`/`isolated`.
- Magnet-only torrents (no trackers) rely on DHT and are much less reliable —
  prefer torrents with trackers, or `.torrent` files.
- The daemon stores its state in `~/.config/transmission-daemon`.

## License

- Transmission: GPL-2.0-or-later (with parts under MIT) — see upstream
  <https://github.com/transmission/transmission>.
- Transmission Daemon UI: MIT, see [LICENSE](LICENSE).
