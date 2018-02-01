#!/bin/bash

apt-get -y install build-essential wget bzip2 ccache cmake git liblog4cxx10v5 liblog4cxx10-dev libmysqlclient-dev libunwind8-dev libev-dev libtbb-dev libboost-all-dev libhwloc-dev binutils-dev libgoogle-perftools-dev  gfortran curl libtool autoconf libcsv-dev libmetis-dev libpapi-dev

# debconf-set-selections <<< 'mysql-server mysql-server/root_password password somerootpass'
# debconf-set-selections <<< 'mysql-server mysql-server/root_password_again password somerootpass'
# apt-get -y install mysql-server

# mysql -uroot -psomerootpass -e "CREATE USER 'vagrant'@'localhost' IDENTIFIED BY 'vagrant';"
# mysql -uroot -psomerootpass -e "GRANT ALL PRIVILEGES ON *.* TO 'vagrant'@'localhost';"
# mysql -uroot -psomerootpass -e "FLUSH PRIVILEGES;"

