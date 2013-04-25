#
# Cookbook Name:: hyrise
# Recipe:: default
#
# Copyright 2013, Hasso Plattner Institute
#
# All rights reserved - Do Not Redistribute
#

packages = %w{ sphinx-common curl binutils-dev bzip2 ccache cmake doxygen doxygen-latex emacs gdb git gnuplot imagemagick liblog4cxx10 liblog4cxx10-dev libmysqlclient-dev libnuma-dev mysql-common libunwind8-dev libunwind8 libev-dev} 

#Install dependencies
packages.each do |pkg|
  package pkg do 
    action :install
    options "--force-yes"
  end
end

# Install manual dependencies
deps = [
  ["libcsv-3.0.1", "http://downloads.sourceforge.net/project/libcsv/libcsv/libcsv-3.0.1/libcsv-3.0.1.tar.gz"],
  ["metis-5.0.2", "http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.0.2.tar.gz"],
  ["numactl-2.0.7","http://oss.sgi.com/projects/libnuma/download/numactl-2.0.7.tar.gz"],
  ["gperftools-2.0", "http://gperftools.googlecode.com/files/gperftools-2.0.tar.gz"],
  ["hwloc-1.6", "http://www.open-mpi.org/software/hwloc/v1.6/downloads/hwloc-1.6.tar.gz"],
  ["papi-5.0.1", "http://icl.cs.utk.edu/projects/papi/downloads/papi-5.0.1.tar.gz"]
]

deps.each do |dep|
  remote_file "#{Chef::Config['file_cache_path']}/#{dep[0]}.tar.gz" do
    source dep[1]  
    action :create_if_missing
  end
end

bash "install-libcsv" do
  user "root"
  cwd Chef::Config[:file_cache_path]
  code <<-EOH
  tar zxf libcsv-3.0.1.tar.gz
  cd libcsv-3.0.1
  make install
  EOH
end

["gperftools-2.0", "hwloc-1.6"].each do |dep|
  bash "install-#{dep}" do
    user "root"
    cwd Chef::Config[:file_cache_path]
    code <<-EOH
    tar zxf #{dep}.tar.gz
    cd #{dep}
    chmod +x configure
    ./configure && make && make install
    EOH
  end
end

bash "install-papi" do
  user "root"
  cwd Chef::Config[:file_cache_path]
  code <<-EOH
  tar zxf papi-5.0.1.tar.gz
  cd papi-5.0.1/src
  chmod +x configure
  ./configure && make && make install
  EOH
end

bash "install-numactl" do
  user "root"
  cwd Chef::Config[:file_cache_path]
  code <<-EOH
  tar zxf numactl-2.0.7.tar.gz
  cd numactl-2.0.7
  make && make install
  EOH
end

bash "install-metis" do
  user "root"
  cwd Chef::Config[:file_cache_path]
  code <<-EOH
  tar zxf metis-5.0.2.tar.gz
  cd metis-5.0.2
  make config
  make install
  EOH
end

bash "link-dir" do
  user "vagrant"
  cwd "/home/vagrant"
  code <<-EOH
  ln -s /vagrant /home/vagrant/work
  EOH
  only_if { !File.exist?("/home/vagrant/work") }
end

bash "remove-old-stuff" do 
  
  user "vagrant"
  cwd "/home/vagrant"
  
  code <<-EOH
  rm postinstall.sh || echo 1
  rm VBoxGuestAdditions_4.2.2.iso || echo 1
  EOH

end
