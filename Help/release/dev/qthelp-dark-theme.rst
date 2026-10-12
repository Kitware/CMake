qthelp-dark-theme
-----------------

* The Qt Help (``.qch``) rendition of the documentation now carries its own
  color scheme.  Previously it was rendered with layout rules only, because
  the ``qthelp`` builder does not generate the color scheme that the HTML
  documentation uses.  A dark variant is shipped alongside it, which help
  viewers such as Qt Creator use when their own theme is dark.
