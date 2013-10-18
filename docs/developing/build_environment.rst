***********************
Setup Build Environment
***********************

Initial Git Setup
=================

Make sure git is correctly configured and an ssh key is setup to be
used with `Github <https://epic.plan.io/projects/hyrise>`_::

    # download and install Git
    git config --global user.name "Johannes Wust"
    git config --global user.email johannes.wust@hpi.uni-potsdam.de

    git clone https://github.com/hyrise/hyrise.git

    git submodule update --init


Build HYRISE
============

Run ``make`` to compile HYRISE. Further instructions can be found in :doc:`building`.
