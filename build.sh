#!/bin/sh
# Build the merged transmission-daemon package with the Sailfish OS SDK.
set -eu
cd "$(dirname "$0")"
rm -rf RPMS
sudo /srv/mer/sdks/sfossdk/mer-sdk-chroot -m all -- \
	bash -c "cd $(pwd) && mb2 build"
ls -l RPMS/*.rpm
