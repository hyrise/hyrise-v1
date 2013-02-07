#
# Cookbook Name:: nfs
# Recipe:: server 
#
# Copyright 2011, Eric G. Wolfe
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include_recipe "nfs"

# Install server components for Debian
case node['platform']
when "debian","ubuntu"
  package "nfs-kernel-server"
end

# Start nfs-server components
service node['nfs']['service']['server'] do
  action [ :start, :enable ]
  supports :status => true
end

# Configure nfs-server components
template node['nfs']['config']['server_template'] do
  mode 0644
  notifies :restart, resources(:service => node['nfs']['service']['server'])
end

