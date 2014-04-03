##########
Repository
##########


Obtaining the code
==================

The primary method of obtaining the git repository is through the github_ 
project site (You need an account). The code can then be obtained through::

    git clone git@github.com:hyrise/hyrise.git

.. _github: https://github.com/hyrise/hyrise


Flow
====

We encourage the branching model imposed by git-flow_. It should be obtainable 
through your favorite package manager (homebrew, apt...). We encourage the use 
as follows::

    # In hyrise project checkout
    # Create a new release branch
    $ git checkout -b release
    Switched to a new branch 'release'
    $ git checkout master
    Switched to branch 'master'
    $ git flow init

    Which branch should be used for bringing forth production releases?
       - master
       - release
    Branch name for production releases: [master] release

    Which branch should be used for integration of the "next release"?
       - master
    Branch name for "next release" development: [master] 

    How to name your supporting branch prefixes?
    Feature branches? [feature/] 
    Release branches? [release/] 
    Hotfix branches? [hotfix/] 
    Support branches? [support/] 
    Version tag prefix? [] 
    

This should allow you to use git flow as expected by the project.

.. _git-flow : http://jeffkreeftmeijer.com/2010/why-arent-you-using-git-flow/


Branching
=========

Before merging your changes into the *master* branch, make sure that
you reviewed the code together with one of the product owners (Martin,
Jens, Johannes, Florian). 

If your work on a new bug, feature or hotfix, please use git-flow to correctly
mark your branches with also the associated ticket number::

    $ git flow feature start t<number>_name_of_ticket

Once finished::

    $ git flow feature finish <featurename>