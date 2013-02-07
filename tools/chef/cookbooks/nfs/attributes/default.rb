#
# Cookbook Name:: nfs
# Attributes:: default
#
# Copyright 2011, Eric G. Wolfe
#
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Default options are taken from the Debian guide on static NFS ports
default['nfs']['port']['statd'] = 32765
default['nfs']['port']['statd_out'] = 32766
default['nfs']['port']['mountd'] = 32767
default['nfs']['port']['lockd'] = 32768

# Default options are based on RHEL5, as the attribute names were
# adopted from this platform.
default['nfs']['packages'] = %w{ nfs-utils portmap }
default['nfs']['service']['portmap'] = "portmap"
default['nfs']['service']['lock'] = "nfslock"
default['nfs']['service']['server'] = "nfs"
default['nfs']['config']['client_templates'] = %w{ /etc/sysconfig/nfs }
default['nfs']['config']['server_template'] = "/etc/sysconfig/nfs"

case node['platform_family']
when "rhel"
  # RHEL6 edge case package set and portmap name 
  if node['platform_version'].to_i >= 6
    default['nfs']['packages'] = %w{ nfs-utils rpcbind }
    default['nfs']['service']['portmap'] = "rpcbind"
  end
when "debian"
  default['nfs']['packages'] = %w{ nfs-common portmap }
  default['nfs']['service']['portmap'] = "portmap"
  default['nfs']['service']['lock'] = "statd"
  default['nfs']['service']['server'] = "nfs-kernel-server"
  default['nfs']['config']['client_templates'] = %w{ /etc/default/nfs-common /etc/modprobe.d/lockd.conf }
  default['nfs']['config']['server_template'] = "/etc/default/nfs-kernel-server"

  # Ubuntu 11.04 edge case package set and portmap name
  if node['platform_version'].to_f == 11.04
    default['nfs']['service']['portmap'] = "rpcbind"
    default['nfs']['packages'] = %w{ nfs-common rpcbind }
  # Ubuntu 11.10 edge case package set and portmap name
  elsif node['platform_version'].to_f >= 11.10 
    default['nfs']['service']['portmap'] = "rpcbind-boot"
    default['nfs']['packages'] = %w{ nfs-common rpcbind }
  end

  # Debian 6.0+
  if node['platform'] == "debian" and node['platform_version'].to_i >= 6
    default['nfs']['service']['lock'] = "nfs-common"
  end
end
