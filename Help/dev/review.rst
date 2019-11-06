CMake Review Process
********************

The following documents the process for reviewing and integrating changes.
See `CONTRIBUTING.rst`_ for instructions to contribute changes.
See documentation on `CMake Development`_ for more information.

.. _`CONTRIBUTING.rst`: ../../CONTRIBUTING.rst
.. _`CMake Development`: README.rst

.. contents:: The review process consists of the following steps:

Merge Request
=============

A user initiates the review process for a change by pushing a *topic
branch* to his or her own fork of the `CMake Repository`_ on GitLab and
creating a *merge request* ("MR").  The new MR will appear on the
`CMake Merge Requests Page`_.  The rest of the review and integration
process is managed by the merge request page for the change.

During the review process, the MR submitter should address review comments
or test failures by updating the MR with a (force-)push of the topic
branch.  The update initiates a new round of review.

We recommend that users enable the "Remove source branch when merge
request is accepted" option when creating the MR or by editing it.
This will cause the MR topic branch to be automatically removed from
the user's fork during the `Merge`_ step.

.. _`CMake Merge Requests Page`: https://gitlab.kitware.com/cmake/cmake/merge_requests
.. _`CMake Repository`: https://gitlab.kitware.com/cmake/cmake

Workflow Status
---------------

`CMake GitLab Project Developers`_ may set one of the following labels
in GitLab to track the state of a MR:

* ``workflow:wip`` indicates that the MR needs additional updates from
  the MR submitter before further review.  Use this label after making
  comments that require such updates.

* ``workflow:in-review`` indicates that the MR awaits feedback from a
  human reviewer or from `Topic Testing`_.  Use this label after making
  comments requesting such feedback.

* ``workflow:nightly-testing`` indicates that the MR awaits results
  of `Integration Testing`_.  Use this label after making comments
  requesting such staging.

* ``workflow:expired`` indicates that the MR has been closed due
  to a period of inactivity.  See the `Expire`_ step.  Use this label
  after closing a MR for this reason.

* ``workflow:external-discussion`` indicates that the MR has been closed
  pending discussion elsewhere.  See the `External Discussion`_ step.
  Use this label after closing a MR for this reason.

The workflow status labels are intended to be mutually exclusive,
so please remove any existing workflow label when adding one.

.. _`CMake GitLab Project Developers`: https://gitlab.kitware.com/cmake/cmake/settings/members

Robot Review
============

The "Kitware Robot" (``@kwrobot``) automatically performs basic checks on
the commits proposed in a MR.  If all is well the robot silently reports
a successful "build" status to GitLab.  Otherwise the robot posts a comment
with its diagnostics.  **A topic may not be merged until the automatic
review succeeds.**

Note that the MR submitter is expected to address the robot's comments by
*rewriting* the commits named by the robot's diagnostics (e.g., via
``git rebase -i``). This is because the robot checks each commit individually,
not the topic as a whole. This is done in order to ensure that commits in the
middle of a topic do not, for example, add a giant file which is then later
removed in the topic.

Automatic Check
---------------

The automatic check is repeated whenever the topic branch is updated.
One may explicitly request a re-check by adding a comment with the
following command among the `comment trailing lines`_::

  Do: check

``@kwrobot`` will add an award emoji to the comment to indicate that it
was processed and also run its checks again.

Automatic Format
----------------

The automatic check will reject commits introducing source code not
formatted according to ``clang-format``.  One may ask the robot to
automatically rewrite the MR topic branch with expected formatting
by adding a comment with the following command among the
`comment trailing lines`_::

  Do: reformat

``@kwrobot`` will add an award emoji to the comment to indicate that it
was processed and also rewrite the MR topic branch and force-push an
updated version with every commit formatted as expected by the check.

Human Review
============

Anyone is welcome to review merge requests and make comments!

Please make comments directly on the MR page Discussion and Changes tabs
and not on individual commits.  Comments on a commit may disappear
from the MR page if the commit is rewritten in response.

Reviewers may add comments providing feedback or to acknowledge their
approval.  Lines of specific forms will be extracted during the `merge`_
step and included as trailing lines of the generated merge commit message.
Each review comment consists of up to two parts which must be specified
in the following order: `comment body`_, then `comment trailing lines`_.
Each part is optional, but they must be specified in this order.

Comment Body
------------

The body of a comment may be free-form `GitLab Flavored Markdown`_.
See GitLab documentation on `Special GitLab References`_ to add links to
things like issues, commits, or other merge requests (even across projects).

Additionally, a line in the comment body may start with one of the
following votes:

* ``-1`` or ``:-1:`` indicates "the change is not ready for integration".

* ``+1`` or ``:+1:`` indicates "I like the change".
  This adds an ``Acked-by:`` trailer to the `merge`_ commit message.

* ``+2`` indicates "the change is ready for integration".
  This adds a ``Reviewed-by:`` trailer to the `merge`_ commit message.

* ``+3`` indicates "I have tested the change and verified it works".
  This adds a ``Tested-by:`` trailer to the `merge`_ commit message.

.. _`GitLab Flavored Markdown`: https://gitlab.kitware.com/help/user/markdown.md
.. _`Special GitLab References`: https://gitlab.kitware.com/help/user/markdown.md#special-gitlab-references

Comment Trailing Lines
----------------------

Zero or more *trailing* lines in the last section of a comment may appear
with the form ``Key: Value``.  The first such line should be separated
from a preceding `comment body`_ by a blank line.  Any key-value pair(s)
may be specified for human reference.  A few specific keys have meaning to
``@kwrobot`` as follows.

Comment Trailer Votes
^^^^^^^^^^^^^^^^^^^^^

Among the `comment trailing lines`_ one may cast a vote using one of the
following pairs followed by nothing but whitespace before the end of the line:

* ``Rejected-by: me`` indicates "the change is not ready for integration".
* ``Acked-by: me`` indicates "I like the change".
  This adds an ``Acked-by:`` trailer to the `merge`_ commit message.
* ``Reviewed-by: me`` indicates "the change is ready for integration".
  This adds a ``Reviewed-by:`` trailer to the `merge`_ commit message.
* ``Tested-by: me`` indicates "I have tested the change and verified it works".
  This adds a ``Tested-by:`` trailer to the `merge`_ commit message.

Each ``me`` reference may instead be an ``@username`` reference or a full
``Real Name <user@domain>`` reference to credit someone else for performing
the review.  References to ``me`` and ``@username`` will automatically be
transformed into a real name and email address according to the user's
GitLab account profile.

Comment Trailer Commands
^^^^^^^^^^^^^^^^^^^^^^^^

Among the `comment trailing lines`_ authorized users may issue special
commands to ``@kwrobot`` using the form ``Do: ...``:

* ``Do: check`` explicitly re-runs the robot `Automatic Check`_.
* ``Do: reformat`` rewrites the MR topic for `Automatic Format`_.
* ``Do: test`` submits the MR for `Topic Testing`_.
* ``Do: stage`` submits the MR for `Integration Testing`_.
* ``Do: merge`` submits the MR for `Merge`_.

See the corresponding sections for details on permissions and options
for each command.

Commit Messages
---------------

Part of the human review is to check that each commit message is appropriate.
The first line of the message should begin with one or two words indicating the
area the commit applies to, followed by a colon and then a brief summary.
Committers should aim to keep this first line short. Any subsequent lines
should be separated from the first by a blank line and provide relevant, useful
information.

Area Prefix on Commit Messages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The appropriateness of the initial word describing the area the commit applies
to is not something the automatic robot review can judge, so it is up to the
human reviewer to confirm that the area is specified and that it is
appropriate. Good area words include the module name the commit is primarily
fixing, the main C++ source file being edited, ``Help`` for generic
documentation changes or a feature or functionality theme the changes apply to
(e.g. ``server`` or ``Autogen``). Examples of suitable first lines of a commit
message include:

* ``Help: Fix example in cmake-buildsystem(7) manual``
* ``FindBoost: Add support for 1.64``
* ``Autogen: Extended mocInclude tests``
* ``cmLocalGenerator: Explain standard flag selection logic in comments``

Referencing Issues in Commit Messages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the commit fixes a particular reported issue, this information should
ideally also be part of the commit message. The recommended way to do this is
to place a line at the end of the message in the form ``Fixes: #xxxxx`` where
``xxxxx`` is the GitLab issue number and to separate it from the rest of the
text by a blank line. For example::

  Help: Fix FooBar example robustness issue

  FooBar supports option X, but the example provided
  would not work if Y was also specified.

  Fixes: #12345

GitLab will automatically create relevant links to the merge request and will
close the issue when the commit is merged into master. GitLab understands a few
other synonyms for ``Fixes`` and allows much more flexible forms than the
above, but committers should aim for this format for consistency. Note that
such details can alternatively be specified in the merge request description.

Referencing Commits in Commit Messages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The preferred form for references to other commits is
``commit <shorthash> (<subject>, <date>)``, where:

* ``<shorthash>``:
  The abbreviated hash of the commit.

* ``<subject>``:
  The first line of the commit message.

* ``<date>``:
  The author date of the commit, in its original time zone, formatted as
  ``CCYY-MM-DD``.  ``git-log(1)`` shows the original time zone by default.

This may be generated with
``git show -s --date=short --pretty="format:%h (%s, %ad)" <commit>``.

If the commit is a fix for the mentioned commit, consider using a ``Fixes:``
trailer in the commit message with the specified format. This trailer should
not be word-wrapped. Note that if there is also an issue for what is being
fixed, it is preferrable to link to the issue instead.

If relevant, add the first release tag of CMake containing the commit after
the ``<date>``, i.e., ``commit <shorthash> (<subject>, <date>, <tag>)``.

Alternatively, the full commit ``<hash>`` may be used.

Revising Commit Messages
^^^^^^^^^^^^^^^^^^^^^^^^

Reviewers are encouraged to ask the committer to amend commit messages to
follow these guidelines, but prefer to focus on the changes themselves as a
first priority. Maintainers will also make a check of commit messages before
merging.

Topic Testing
=============

CMake has a `buildbot`_ instance watching for merge requests to test.
`CMake GitLab Project Developers`_ may activate buildbot on a MR by
adding a comment with a command among the `comment trailing lines`_::

  Do: test

``@kwrobot`` will add an award emoji to the comment to indicate that it
was processed and also inform buildbot about the request.  The buildbot
user (``@buildbot``) will schedule builds and respond with a comment
linking to the `CMake CDash Page`_ with a filter for results associated
with the topic test request.  If the MR topic branch is updated by a
push a new ``Do: test`` command is needed to activate testing again.

The ``Do: test`` command accepts the following arguments:

* ``--stop``: clear the list of commands for the merge request
* ``--clear``: clear previous commands before adding this command
* ``--regex-include <arg>`` or ``-i <arg>``: only build on builders
  matching ``<arg>`` (a Python regular expression)
* ``--regex-exclude <arg>`` or ``-e <arg>``: exclude builds on builders
  matching ``<arg>`` (a Python regular expression)

Builder names follow the pattern ``project-host-os-buildtype-generator``:

* ``project``: always ``cmake`` for CMake builds
* ``host``: the buildbot host
* ``os``: one of ``windows``, ``osx``, or ``linux``
* ``buildtype``: ``release`` or ``debug``
* ``generator``: ``ninja``, ``makefiles``, ``vs<year>``,
  or ``lint-iwyu-tidy``

The special ``lint-<tools>`` generator name is a builder that builds
CMake using lint tools but does not run the test suite (so the actual
generator does not matter).

.. _`buildbot`: http://buildbot.net
.. _`CMake CDash Page`: https://open.cdash.org/index.php?project=CMake

Integration Testing
===================

The above `topic testing`_ tests the MR topic independent of other
merge requests and on only a few key platforms and configurations.
The `CMake Testing Process`_ also has a large number of machines
provided by Kitware and generous volunteers that cover nearly all
supported platforms, generators, and configurations.  In order to
avoid overwhelming these resources, they do not test every MR
individually.  Instead, these machines follow an *integration branch*,
run tests on a nightly basis (or continuously during the day), and
post to the `CMake CDash Page`_.  Some follow ``master``.  Most follow
a special integration branch, the *topic stage*.

The topic stage is a special branch maintained by the "Kitware Robot"
(``@kwrobot``).  It consists of the head of the MR target integration
branch (e.g. ``master``) branch followed by a sequence of merges each
integrating changes from an open MR that has been staged for integration
testing.  Each time the target integration branch is updated the stage
is rebuilt automatically by merging the staged MR topics again.
The branch is stored in the upstream repository by special refs:

* ``refs/stage/master/head``: The current topic stage branch.
  This is used by continuous builds that report to CDash.
* ``refs/stage/master/nightly/latest``: Topic stage as of 1am UTC each night.
  This is used by most nightly builds that report to CDash.
* ``refs/stage/master/nightly/<yyyy>/<mm>/<dd>``: Topic stage as of 1am UTC
  on the date specified. This is used for historical reference.

`CMake GitLab Project Developers`_ may stage a MR for integration testing
by adding a comment with a command among the `comment trailing lines`_::

  Do: stage

``@kwrobot`` will add an award emoji to the comment to indicate that it
was processed and also attempt to add the MR topic branch to the topic
stage.  If the MR cannot be added (e.g. due to conflicts) the robot will
post a comment explaining what went wrong.

Once a MR has been added to the topic stage it will remain on the stage
until one of the following occurs:

* The MR topic branch is updated by a push.

* The MR target integration branch (e.g. ``master``) branch is updated
  and the MR cannot be merged into the topic stage again due to conflicts.

* A developer or the submitter posts an explicit ``Do: unstage`` command.
  This is useful to remove a MR from the topic stage when one is not ready
  to push an update to the MR topic branch.  It is unnecessary to explicitly
  unstage just before or after pushing an update because the push will cause
  the MR to be unstaged automatically.

* The MR is closed.

* The MR is merged.

Once a MR has been removed from the topic stage a new ``Do: stage``
command is needed to stage it again.

.. _`CMake Testing Process`: testing.rst

Resolve
=======

The workflow used by the CMake project supports a number of different
ways in which a MR can be moved to a resolved state.  In addition to
the conventional practices of merging or closing a MR without merging it,
a MR can also be moved to a quasi-resolved state pending some action.
This may involve moving discussion to an issue or it may be the result of
an extended period of inactivity.  These quasi-resolved states are used
to help manage the relatively large number of MRs the project receives
and are not an indication of the changes being rejected.  The following
sections explain the different resolutions a MR may be given.

Merge
-----

Once review has concluded that the MR topic is ready for integration,
`CMake GitLab Project Masters`_ may merge the topic by adding a comment
with a command among the `comment trailing lines`_::

  Do: merge

``@kwrobot`` will add an award emoji to the comment to indicate that it
was processed and also attempt to merge the MR topic branch to the MR
target integration branch (e.g. ``master``).  If the MR cannot be merged
(e.g. due to conflicts) the robot will post a comment explaining what
went wrong.  If the MR is merged the robot will also remove the source
branch from the user's fork if the corresponding MR option was checked.

The robot automatically constructs a merge commit message of the following
form::

  Merge topic 'mr-topic-branch-name'

  00000000 commit message subject line (one line per commit)

  Acked-by: Kitware Robot <kwrobot@kitware.com>
  Merge-request: !0000

Mention of the commit short sha1s and MR number helps GitLab link the
commits back to the merge request and indicates when they were merged.
The ``Acked-by:`` trailer shown indicates that `Robot Review`_ passed.
Additional ``Acked-by:``, ``Reviewed-by:``, and similar trailers may be
collected from `Human Review`_ comments that have been made since the
last time the MR topic branch was updated with a push.

The ``Do: merge`` command accepts the following arguments:

* ``-t <topic>``: substitute ``<topic>`` for the name of the MR topic
  branch in the constructed merge commit message.

Additionally, ``Do: merge`` extracts configuration from trailing lines
in the MR description (the following have no effect if used in a MR
comment instead):

* ``Backport: release[:<commit-ish>]``: merge the topic branch into
  the ``release`` branch to backport the change.  This is allowed
  only if the topic branch is based on a commit in ``release`` already.
  If only part of the topic branch should be backported, specify it as
  ``:<commit-ish>``.  The ``<commit-ish>`` may use `git rev-parse`_
  syntax to reference commits relative to the topic ``HEAD``.
  See additional `backport instructions`_ for details.
  For example:

  ``Backport: release``
    Merge the topic branch head into both ``release`` and ``master``.
  ``Backport: release:HEAD~1^2``
    Merge the topic branch head's parent's second parent commit into
    the ``release`` branch.  Merge the topic branch head to ``master``.

* ``Topic-rename: <topic>``: substitute ``<topic>`` for the name of
  the MR topic branch in the constructed merge commit message.
  It is also used in merge commits constructed by ``Do: stage``.
  The ``-t`` option to a ``Do: merge`` command overrides any topic
  rename set in the MR description.

.. _`CMake GitLab Project Masters`: https://gitlab.kitware.com/cmake/cmake/settings/members
.. _`backport instructions`: https://gitlab.kitware.com/utils/git-workflow/wikis/Backport-topics
.. _`git rev-parse`: https://git-scm.com/docs/git-rev-parse

Close
-----

If review has concluded that the MR should not be integrated then it
may be closed through GitLab.  This would normally be a final state
with no expectation that the MR would be re-opened in the future.
It is also used when a MR is being superseded by another separate one,
in which case a reference to the new MR should be added to the MR being
closed.

Expire
------

If progress on a MR has stalled for a while, it may be closed with a
``workflow:expired`` label and a comment indicating that the MR has
been closed due to inactivity (it may also be done where the MR is blocked
for an extended period by work in a different MR).  This is not an
indication that there is a problem with the MR's content, it is only a
practical measure to allow the reviewers to focus attention on MRs that
are actively being worked on.  As a guide, the average period of inactivity
before transitioning a MR to the expired state would be around 2 weeks,
but this may decrease to 1 week or less when there is a high number of
open merge requests.

Reviewers would usually provide a message similar to the following when
resolving a MR as expired::

  Closing for now. @<MR-author> please re-open when ready to continue work.

This is to make it clear to contributors that they are welcome to re-open
the expired MR when they are ready to return to working on it and moving
it forward.  In the meantime, the MR will appear as ``Closed`` in GitLab,
but it can be differentiated from permanently closed MRs by the presence
of the ``workflow:expired`` label.

**NOTE:** Please re-open *before* pushing an update to the MR topic branch
to ensure GitLab will still act on the association.  If changes are pushed
before re-opening the MR, the reviewer should initiate a ``Do: check`` to
force GitLab to act on the updates.

External Discussion
-------------------

In some situations, a series of comments on a MR may develop into a more
involved discussion, or it may become apparent that there are broader
discussions that need to take place before the MR can move forward in an
agreed direction.  Such discussions are better suited to GitLab issues
rather than in a MR because MRs may be superseded by a different MR, or
the set of changes may evolve to look quite different to the context in
which the discussions began.  When this occurs, reviewers may ask the
MR author to open an issue to discuss things there and they will transition
the MR to a resolved state with the label ``workflow:external-discussion``.
The MR will appear in GitLab as closed, but it can be differentiated from
permanently closed MRs by the presence of the ``workflow:external-discussion``
label.  Reviewers should leave a message clearly explaining the action
so that the MR author understands that the MR closure is temporary and
it is clear what actions need to happen next.  The following is an example
of such a message, but it will usually be necessary to tailor the message
to the individual situation::

  The desired behavior here looks to be more involved than first thought.
  Please open an issue so we can discuss the relevant details there.
  Once the path forward is clear, we can re-open this MR and continue work.

When the discussion in the associated issue runs its course and the way
forward is clear, the MR can be re-opened again and the
``workflow:external-discussion`` label removed.  Reviewers should ensure
that the issue created contains a reference to the MR so that GitLab
provides a cross-reference to link the two.
