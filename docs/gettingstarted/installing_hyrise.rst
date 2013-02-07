=================
Installing Hyrise
=================

Linux(Ubuntu) - Initial Setup
-----------------------------

Install base requisites via:: 
        
   sudo apt-get install openssh-server git-core

Initial git setup::
    
    git config --global user.name "Your Name"
    git config --global user.email your@emailAdress.com
    
If you don't have your own account at epic.plan.io, please contact `Martin Grund <mailto:martin.grund@hpi.uni-potsdam.de>`_.
    
Set up ssh key with `Planio <https://epic.plan.io/projects/hyrise>`_.

1. Generate a private key using: ``ssh-keygen -t rsa -b 2048``

2. Go to `Planio <https://epic.plan.io/projects/hyrise>`_ -> Mein Konto -> Öffentliche Schlüssel

3. Insert the contents of the file ``~/.ssh/id_rsa.pub`` (public key) to your account

Now you can create your own copy of hyrise on your local disk. This command will create a new folder "hyrise" containing the project.::
    
    git clone git@epic.plan.io:epic-hyrise.git hyrise

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
-----------

Having build HYRISE, you should now head over to :doc:`json_queries`

