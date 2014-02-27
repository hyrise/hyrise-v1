########################
HYRISE & Development FAQ
########################

This is a list of Frequently Asked Questions about HYRISE. Feel free
to suggest new entries

HYRISE Related
==============

How Do I ...

... enable the production build?

    You have to set the environment variable ``PRODUCTION``

Git Related
===========

How Do I ...


... discard all my local local changed and force a reset to the remote master?

    You have to use the ``reset`` featurre of git. The easiest way is
    to execute the command ``git reset --hard origin/master``

... remove unused remote branches that no longer exist?

    ``git remote prune origin``

... delete a remote branch?

    ``git push origin :branch_to_delete``

... start a new feature branch?

    Make sure `git-flow <https://github.com/nvie/gitflow>`_ is
    installed and follow the instructions. Now execute ``git flow
    feature start my_feature``.
    
.. toctree::
       :glob: