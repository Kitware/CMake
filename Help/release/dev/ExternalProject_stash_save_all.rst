ExternalProject_stash_save_all
------------------------------

* Use "git stash save --all" only if supported. The --all option for git-stash
  wasn't introduced until git version 1.7.6.
