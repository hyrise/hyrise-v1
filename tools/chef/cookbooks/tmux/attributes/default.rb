default['tmux']['install_method'] = if(node['platform_family'] == 'rhel')
  'source'
else
  'package'
end
default['tmux']['version'] = '1.6'
default['tmux']['checksum'] = 'faee08ba1bd8c22537cd5b7458881d1bdb4985df88ed6bc5967c56881a7efbd6'
