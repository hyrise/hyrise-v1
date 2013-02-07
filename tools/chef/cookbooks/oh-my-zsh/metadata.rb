maintainer       "Heavy Water Software Inc."
maintainer_email "darrin@heavywater.ca"
license          "Apache 2.0"
description      "Installs/Configures oh-my-zsh"
long_description IO.read(File.join(File.dirname(__FILE__), 'README.rdoc'))
version          "0.4.2"

depends          "git"
depends          "zsh"
depends          "user"

%w( ubuntu debian
    centos redhat fedora ).each do |os|
  supports os
end
