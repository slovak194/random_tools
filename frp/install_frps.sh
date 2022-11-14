#!/usr/bin/env bash

# determine system arch
ARCH=
if [ "$(uname -m)" == 'x86_64' ]
then
    ARCHIVE=frp_0.45.0_linux_amd64.tar.gz
elif [ "$(uname -m)" == 'aarch64' ]
then
    ARCHIVE=frp_0.45.0_linux_arm64.tar.gz
fi

DOWNLOAD_URL=https://github.com/fatedier/frp/releases/download/v0.45.0/$ARCHIVE

if [ ! $(which wget) ]; then
    echo 'Please install wget package'
    exit 1
fi

if (( $EUID != 0 )); then
    echo "Please run as root"
    exit 1
fi

if [ -z "$1" ]; then
    echo "./install.sh <token>"
    exit 1
fi

cp frps.service /lib/systemd/system/
mkdir -p /opt/frp

pushd /opt/frp
wget $DOWNLOAD_URL
tar --strip-components=1 -zxf $ARCHIVE
rm $ARCHIVE
chmod +x frps

popd

cp frps.ini /opt/frp

sed -i "s/<token>/$1/g" /opt/frp/frps.ini

systemctl enable frps.service
systemctl start frps.service

echo "Done installing frps"

exit 0
