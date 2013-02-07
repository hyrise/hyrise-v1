include_recipe "build-essential"

remote_file "#{Chef::Config[:file_cache_path]}/#{node['boost']['file']}" do
  source node['boost']['source'] + node['boost']['file']
  mode "0644"
  action :create_if_missing
end

%w{libzip-dev libzip2 libbz2-dev libbz2-1.0}.each do |pkg|
  package pkg do 
    action :install
    options "--force-yes"
  end
end

bash "install-boost" do
  user "root"
  cwd Chef::Config[:file_cache_path]
  code <<-EOH
  tar xzvf #{node['boost']['file']}
  cd #{node['boost']['build_dir']}
  ./bootstrap.sh --without-libraries=python && ./b2 -j3 install
  EOH
end

execute "ldconfig" do
  user "root"
  command "/sbin/ldconfig"
  action :nothing
end

cookbook_file "/etc/ld.so.conf.d/boost.conf" do
  owner "root"
  group "root"
  mode 00644
  source "boost.conf"
  backup false
  notifies :run, "execute[ldconfig]", :immediately
end
