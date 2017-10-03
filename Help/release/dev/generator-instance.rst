generator-instance
------------------

* A :variable:`CMAKE_GENERATOR_INSTANCE` variable was introduced
  to hold the selected instance of the generator's corresponding
  native tools if multiple are available.  Currently no generators
  actually use this, but the infrastructure is in place.
