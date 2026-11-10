# Transmission daemon + Sailfish OS 界面

适用于 **Sailfish OS（aarch64）** 的 Transmission **4.0.6**，打包成**一个 RPM**：
无界面守护进程、命令行工具、它需要的运行库、systemd 用户服务，以及一个控制它的小应用。

以前是两个包（`transmission` 和 `transmission-daemon-ui`），现在合并成一个。
合并后的包 `Obsoletes` 了旧的两个，升级时会自动替换。

![Transmission Daemon 应用图标](ui/icons/transmission-daemon-256.png)

英文文档：[README.md](README.md)

---

## 包里有什么

| 路径 | 内容 |
|---|---|
| `/usr/bin/transmission-daemon` | 无界面 BT 守护进程（RPC 在 `127.0.0.1:9091`） |
| `/usr/bin/transmission-remote` | 远程控制工具 |
| `/usr/bin/transmission-cli` | 命令行客户端 |
| `/usr/bin/transmission-daemon-ui` | Sailfish 应用（启停开关 + 状态） |
| `/usr/lib/libevent-2.1.so.7*` | 内置的 libevent 2.1 运行库（Sailfish OS 没有） |
| `/usr/lib/systemd/user/transmission-daemon.service` | 以用户服务方式运行守护进程 |
| `/usr/share/transmission-daemon-ui/qml/` | 应用的 QML |
| `/usr/share/applications/transmission-daemon-ui.desktop` | 启动器入口 |

## 安装

```sh
rpm -Uvh --nodeps --replacepkgs --ignorearch transmission-daemon-4.0.6-1.aarch64.rpm

systemctl --user daemon-reload
systemctl --user enable --now transmission-daemon
```

安装包见 [Releases](../../releases)。

## 使用

- RPC：`127.0.0.1:9091`，无认证；
- 下载目录：`~/Downloads`；
- 任何 Transmission RPC 客户端都能连（Tremotesf、transmission-remote 等）；
- 应用在启动器里叫 **Transmission Daemon**。

## 为什么这样构建

1. **不用 3.00**：Transmission 3.00 在这台设备上一算种子哈希就崩溃（OpenSSL 3.5）：
   `assertion failed: handle != NULL (crypto-utils-openssl.c:210)` → SIGABRT。
   4.0.6 原生支持 OpenSSL 3。
2. **原生编译**：Sailfish SDK 的交叉环境满足不了 transmission 的 CMake
   （`find_package(CURL)` 路径问题），所以守护进程是**在手机上**编译的
   （gcc 13.4 + cmake 3.31，来自 Jolla 仓库）。`daemon/` 里的二进制就是那次构建的产物。
3. **内置 libevent**：Sailfish OS 仓库没有 libevent，所以把随 transmission 一起编出的
   `libevent-2.1.so.7*` 打进包里。
4. **补齐 miniupnpc**：上游 tarball 里的 `third-party/miniupnpc` 是空目录（git submodule），
   用 `third-party/miniupnp/miniupnpc` 补齐后才能构建。
5. **为什么要有界面**：守护进程是后台服务，普通用户需要一个简单入口来启停和看状态。
   应用通过 `systemctl --user start/stop transmission-daemon` 控制用户服务，
   通过 `127.0.0.1:9091` 的 RPC 读取版本、种子数和上下行速度。
6. **界面关闭沙箱**：Sailfish 5.2 默认把应用放进 sailjail 沙箱，会挡住 `systemctl --user`，
   开关就无法启停服务。所以 `.desktop` 里带了 `[X-Sailjail] Sandboxing=Disabled`。
   这是本地系统工具，接受该取舍；停止也可以走 RPC 的 `session-close`，不需要沙箱权限。

## 构建

界面从源码构建，守护进程用预编译的二进制。

```sh
./build.sh          # 在 Sailfish SDK chroot 里跑 mb2
```

spec（`rpm/transmission-daemon.spec`）编译 `ui/`、安装 `daemon/` 里的预编译文件，
产出 `RPMS/transmission-daemon-4.0.6-1.aarch64.rpm`。

如果要重新编译守护进程本身，按 `daemon/README.sfos` 的说明在设备上构建 transmission 4.0.6，
把产物放进 `daemon/` 即可。

## 注意

- 在受限网络下 **P2P/Tracker 需要走 VPN**，否则种子会一直 `Idle`/`isolated`；
- 纯磁力链（无 tracker）依赖 DHT，连通性差得多，建议用带 tracker 的种子或 `.torrent` 文件；
- 守护进程的状态保存在 `~/.config/transmission-daemon`。

## 许可证

- Transmission：GPL-2.0-or-later（部分 MIT），见上游
  <https://github.com/transmission/transmission>；
- Transmission Daemon UI：MIT，见 [LICENSE](LICENSE)。
