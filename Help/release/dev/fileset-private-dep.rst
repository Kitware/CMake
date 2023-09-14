fileset-private-dep
-------------------

* Generated files, in targets using :ref:`file sets`, are now considered
  private by default.  Generated public headers must be specified using
  file sets.  This allows :ref:`Ninja Generators` to produce more
  efficient build graphs.  See policy :policy:`CMP0154`.
