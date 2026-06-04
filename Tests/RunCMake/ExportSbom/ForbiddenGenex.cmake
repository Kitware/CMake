include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ForbiddenGenex.cmake)
export(SBOM foo
  EXPORTS foo
  FORMAT "spdx-3.0+json"
)
