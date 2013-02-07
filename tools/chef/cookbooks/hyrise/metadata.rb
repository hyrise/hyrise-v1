maintainer       "Hasso Plattner Institute"
maintainer_email "martin.grund@unifr.ch"
license          "All rights reserved"
description      "Installs/Configures HYRISE Dependencies"
long_description IO.read(File.join(File.dirname(__FILE__), 'README.md'))
version          "0.1.0"

depends "git"
depends "user"

%w{ ubuntu debian }.each do |os|
  supports os
end