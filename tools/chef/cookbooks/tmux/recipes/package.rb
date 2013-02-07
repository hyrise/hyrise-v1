if node['platform_family'] == 'rhel'
  include_recipe 'yum::epel'
end

package 'tmux'
