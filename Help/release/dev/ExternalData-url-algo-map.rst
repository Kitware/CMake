ExternalData-url-algo-map
-------------------------

* The :module:`ExternalData` module learned a new URL template
  placeholder ``%(algo:<key>)`` to allow custom mapping from
  algorithm name to URL component through configuration of new
  :variable:`ExternalData_URL_ALGO_<algo>_<key>` variables.
  This allows more flexibility in remote URLs.
