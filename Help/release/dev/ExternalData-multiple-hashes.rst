ExternalData-multiple-hashes
----------------------------

* The :module:`ExternalData` module learned to support multiple
  content links for one data file using different hashes, e.g.
  ``img.png.sha256`` and ``img.png.sha1``.  This allows objects
  to be fetched from sources indexed by different hash algorithms.
