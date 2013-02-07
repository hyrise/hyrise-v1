# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant::Config.run do |config|
  config.vm.box = "quantal64"
  config.vm.share_folder("v-root", "/vagrant", ".", :nfs => true, :nfs_version => 3)
  #config.vm.boot_mode = "gui"
  
  config.vm.customize [
    "modifyvm", :id,
    "--memory", "4096",
    "--name", "ubuntu",
    "--nicspeed1", 1000000,
    "--nicspeed2", 1000000,
    "--cpus", "2"
  ]

  config.vm.box_url = "https://github.com/downloads/roderik/VagrantQuantal64Box/quantal64.box"
  config.vm.network :hostonly, "192.168.200.10"

  config.vm.provision :chef_solo do |chef|
    # Debug information
    chef.log_level = :info

    # Configuraiton
    chef.cookbooks_path = "tools/chef/cookbooks"
    chef.data_bags_path = "tools/chef/data_bags"
    chef.roles_path = "tools/chef/roles"
    
    # Recipes
    chef.add_recipe("apt")
    chef.add_recipe("build-essential")
    chef.add_recipe("nfs")
    chef.add_recipe("nfs::server")
    chef.add_recipe("boost")
    chef.add_recipe("tmux")
    
    chef.json = {
      "mysql" => {
        "server_root_password" => "root",
        "server_repl_password" => "root",
        "server_debian_password" => "root"
      }
    }
    chef.add_recipe("mysql::server")   
    chef.add_recipe("hyrise")

    # The following packages are more or less optional but 
    # recommended for development
    chef.add_recipe("oh-my-zsh") 
    chef.add_recipe("hyrise::changetozsh")
    chef.add_recipe("hyrise::valgrind")
    chef.add_recipe("hyrise::gccsnapshot")
    chef.add_recipe("hyrise::scmbreeze")
    chef.add_recipe("hyrise::gccfilter")
  end



end
