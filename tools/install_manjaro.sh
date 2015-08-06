sudo pacman -S base-devel wget bzip2 ccache cmake git libunwind liblogging libev hwloc gcc-fortran boost intel-tbb binutils curl yaourt

yaourt -S libcsv metis papi gperftools log4cxx

sudo mkdir /usr/include/libcsv
sudo ln -s /usr/include/csv.h /usr/include/libcsv/csv.h