#!/bin/sh

BASEDIR=$(dirname $0)

apt-get install build-essential binutils-doc autoconf libboost-all-dev curl bzip2 ccache cmake liblog4cxx10-dev libnuma-dev libev-dev libhwloc-dev libbfd-dev

sh -ex $BASEDIR/hyriseExternalDeps.sh
