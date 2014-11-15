
set -x
set -e


apt-get -y install libboost-filesystem1.54-dev libboost-program-options1.54-dev libboost-regex1.54-dev libboost-system1.54-dev libboost1.54-dev

apt-get -y install build-essential wget bzip2 ccache cmake git liblog4cxx10 liblog4cxx10-dev libmysqlclient-dev libunwind8-dev libev-dev libtbb-dev libhwloc-dev binutils-dev libgoogle-perftools-dev  gfortran curl git

mkdir -p dependencies
cd dependencies

# install libcsv 
#########################

if [ ! -e libcsv-3.0.1 ]; then
  wget http://downloads.sourceforge.net/project/libcsv/libcsv/libcsv-3.0.1/libcsv-3.0.1.tar.gz
  tar -xf libcsv-3.0.1.tar.gz
  cd libcsv-3.0.1
  make -j 4
  cd ..
fi

cd libcsv-3.0.1
make install
cd ..


# install metis 
#########################

if [ ! -e metis-5.1.0 ]; then
  wget http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.1.0.tar.gz
  tar -xf metis-5.1.0.tar.gz
  cd metis-5.1.0
  make config
  make -j 4
  cd ..
fi

cd metis-5.1.0
make install
cd ..

# install papi 
#########################

if [ ! -e papi-5.3.0 ]; then
  wget http://icl.cs.utk.edu/projects/papi/downloads/papi-5.3.0.tar.gz
  tar -xf papi-5.3.0.tar.gz
  cd papi-5.3.0/src
  ./configure
  make -j 4
  cd ../..
fi

cd papi-5.3.0/src
make install
cd ..


# go back
################
cd ..



# debconf-set-selections <<< 'mysql-server mysql-server/root_password password somerootpass'
# debconf-set-selections <<< 'mysql-server mysql-server/root_password_again password somerootpass'
# apt-get -y install mysql-server

# mysql -uroot -psomerootpass -e "CREATE USER 'vagrant'@'localhost' IDENTIFIED BY 'vagrant';"
# mysql -uroot -psomerootpass -e "GRANT ALL PRIVILEGES ON *.* TO 'vagrant'@'localhost';"
# mysql -uroot -psomerootpass -e "FLUSH PRIVILEGES;"
