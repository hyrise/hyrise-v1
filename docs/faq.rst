########################
HYRISE & Development FAQ
########################

This is a list of Frequently Asked Questions about HYRISE. Feel free
to suggest new entries

HYRISE Related
==============

How Do I ...

... enable the produciton build?

    You have to set the environment variable ``PRODUCTION``

... check for memroy leaks when using ``RetainCountingObjects``?  

    The easiest way is to write a test that reperesents the part of
    the code where you suspect the error to be. Now you set the
    envrionment variables ``RETAIN_DEBUG`` and ``RETAIN_DETAIL`` to
    ``1`` and rebuild the complete project. If you now execute the
    test a special retain check is performed during the tear down of
    each test so that at the end of the test is checkd if there are
    any alive objects.


Where can I find...

... and old branch I used to work on, that is no longer in the Planio Repository?

    To compact the size of the repository we had to move out older
    unsused branches to be able to rewrite history and actually
    compact the project. The new remote is:
    ``hyrise_old_branches@svn.hpi.uni-potsdam.de:hyrise_old_branches.git``

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