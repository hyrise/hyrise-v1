bash "install-scmbreeze" do 
	user "vagrant"
	group "vagrant"
	cwd "/home/vagrant"
	environment ({'HOME' => '/home/vagrant'})
	code <<-EOH
	git clone git://github.com/ndbroadbent/scm_breeze.git .scm_breeze
	./.scm_breeze/install.sh
	EOH
end


