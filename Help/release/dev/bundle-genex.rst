bundle-genex
------------

* Two new informational generator expressions to retrieve Apple Bundle
  directories have been added. The first one ``$<TARGET_BUNDLE_DIR:tgt>``
  outputs the full path to the Bundle directory, the other one
  ``$<TARGET_BUNDLE_CONTENT_DIR:tgt>`` outputs the full path to the
  ``Contents`` directory of macOS Bundles and App Bundles. For all other
  bundle types and SDKs it is identical with ``$<TARGET_BUNDLE_DIR:tgt>``.

  Those new expressions are helpful to query Bundle locations independent of
  the different Bundle types and layouts on macOS and iOS.
