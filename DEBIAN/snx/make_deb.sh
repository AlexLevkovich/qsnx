#!/usr/bin/bash

old_dir=`pwd`
cd `dirname $0`
basedir=`pwd`
cd $old_dir

set -o pipefail
shopt -s lastpipe
shopt -s extglob

die() {
    echo $1
    exit 1
}

cd $basedir
[[ -f "$basedir/control" ]] || die "No control file found!"

rm -fR snx_uninstall.sh snx.bin snx
tail -n +`cat snx_install.sh | awk 'BEGIN{i=0}{i++;if (substr($0,1,3) == "BZh") { print i; exit 0}}' 2>/dev/null` snx_install.sh | tar -xj || die "not able to decompress snx_install.sh!"
mv ./snx ./snx.bin || die "not able to rename a snx file!"
cp -f control control.back || die "not able to create a copy of control file!"
version=`./snx.bin -h | grep "build " | awk '{ print $2 }'`
awk -vver=$version '{if ($1 == "Version:") { print sprintf("Version: %s",ver) } else { print $0 }}' control.back > control || die "not able to create a new version of control file!"
rm -f control.back
mkdir -p snx/etc/snx/tmp || die "not able to create a snx/etc/snx/tmp!"
mkdir -p snx/usr/bin || die "not able to create a snx/usr/bin!"
cp -f ./snx.bin snx/usr/bin/snx || die "not able to copy snx.bin"
mkdir -p snx/DEBIAN || die "not able to create a snx/DEBIAN!"
cp -f ./control snx/DEBIAN/control || die "not able to copy control to DEBIAN"
fakeroot dpkg-deb --build snx || die "not able to create a deb file!"
mv snx.deb snx-$version.deb
