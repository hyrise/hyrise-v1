#!/bin/sh

BASEDIR=$(dirname $0)

apt-get install build-essential binutils-doc autoconf flex bison libboost-all-dev curl bzip2 ccache cmake liblog4cxx10 liblog4cxx10-dev libnuma-dev libev-dev

sh -ex $BASEDIR/hyriseExternalDeps.sh
