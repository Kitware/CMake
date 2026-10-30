.. _`CTest Dashboard Step 7`:

Step 7: Authenticated Submissions
=================================

This step extends :ref:`Step 6 <CTest Dashboard Step 6>` by submitting to a
CDash project that requires authentication.

.. literalinclude:: Step7/submit.cmake
  :caption: Step7/submit.cmake
  :language: cmake
  :start-at: Submit to a project

Two options of :command:`ctest_submit` change compared to the previous
steps:

``SUBMIT_URL``
  Overrides the URL from ``CTestConfig.cmake`` to submit to a project that
  requires authentication.

``HTTPHEADER``
  Adds an ``Authorization`` header with a bearer token to the submission.
  CDash checks the token before it accepts the results.

Requirements
^^^^^^^^^^^^

* The same as :ref:`Step 3 <CTest Dashboard Step 3>`.
* A CDash project that requires authentication, and a token that can submit to
  it.

Providing the Token
^^^^^^^^^^^^^^^^^^^

The script reads the token from the ``CDASH_TOKEN`` environment variable.
Never write a token in the script itself:

.. code-block:: shell

  export CDASH_TOKEN=<token>
  ctest -S Step7/submit.cmake -V

In a CI system, store the token as a masked secret variable so that it is
never stored in source control.  Tokens can be created on the
authentication tokens page of a CDash user profile.

Authenticated submissions are useful to:

* Restrict who can submit results to a project.
* Prevent spam or tampering on a shared dashboard.
* Associate submissions with a specific user or service account.
