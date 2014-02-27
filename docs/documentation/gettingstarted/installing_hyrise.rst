#################
Installing Hyrise
#################


Installing Hyrise via Vagrant
=============================

Using `VirtualBox <http://www.virtualbox.org/>`_ and `Vagrant <http://www.vagrantup.com/>`_, you can automatically setup a HYRISE development environment. (This is the recommended way of getting started with HYRISE development).

Check out `this repository <https://github.com/hyrise/hyrise_puppet/>`_ to easily setup a development VM as follows:

::

    # initialize submodules
    $ git submodule update -init

    # then
    $ vagrant up
    [...]
    $ vagrant ssh


Manually installing Hyrise
==========================


Linux(Ubuntu) - Initial Setup
-----------------------------

Install base requisites via:: 
        
   sudo apt-get install openssh-server git-core

Initial git setup::
    
    git config --global user.name "Your Name"
    git config --global user.email your@emailAdress.com
    
Now you can create your own copy of hyrise on your local disk. This command will create a new folder "hyrise" containing the project.::
    
    git clone git@github.com:hyrise/hyrise.git

Now change to the new directory epic-hyrise and initialize and update submodules::
    
    cd hyrise
    git submodule update --init


Linux(Ubuntu) - Installation
----------------------------

For Ubuntu, we provide a set of automatic scripts to install all necessary dependencies::
    
    cd tools/autosetup
    ./install.sh
   

Build the project
-----------------

Go back to the main directory and build the project::
    
    cd ../../
    make
    
You should see something akin to::

    DIR build/lib/helper
    ...


First steps
===========

Having build HYRISE, you should now head over to :doc:`tutorial1_starting_the_server` and :doc:`tutorial2_querying`
