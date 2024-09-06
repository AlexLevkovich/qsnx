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

qmake=/usr/lib/x86_64-linux-gnu/qt5/bin/qmake
proname=qsnx
version=`cat control | grep "Version:" | awk '{print $2}'`
threadcount=`cat /proc/cpuinfo | grep "processor" | tail -n1 | awk '{print $3+1}'`

rmdir -fR $proname
mkdir $proname || die "not able to create $proname dir!"
cd ../..
make distclean
$qmake "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$basedir/$proname" CONFIG+=release CONFIG-=debug || die "qmake returned an error!"
make -j$threadcount || die "make returned an error!"
make install || die "make install returned an error!"
mkdir "$basedir/$proname/DEBIAN" || die "not able to create $basedir/$proname/DEBIAN dir!"
cp -f "$basedir/control" "$basedir/$proname/DEBIAN/control" || die "not able to copy control file!"
cd $basedir
fakeroot dpkg-deb --build $proname || die "not able to create a deb file!"
mv $proname.deb $proname-$version.deb
