.. _reward-reference:

Rewards
=======

Interface
---------
.. autoclass:: ecole.typing.RewardFunction

Listing
-------
The list of reward functions relevant to users is given below.

Is Done
^^^^^^^
.. autoclass:: ecole.reward.IsDone
   :no-members:
   :members: before_reset, extract

LP Iterations
^^^^^^^^^^^^^
.. autoclass:: ecole.reward.LpIterations
   :no-members:
   :members: before_reset, extract

NNodes
^^^^^^
.. autoclass:: ecole.reward.NNodes
   :no-members:
   :members: before_reset, extract

Solving Time
^^^^^^^^^^^^
.. autoclass:: ecole.reward.SolvingTime
   :no-members:
   :members: before_reset, extract


Utilities
---------
The following reward functions are used internally by Ecole.

Constant
^^^^^^^^
.. autoclass:: ecole.reward.Constant
   :no-members:
   :members: before_reset, extract

Arithmetic
^^^^^^^^^^
.. autoclass:: ecole.reward.Arithmetic
   :no-members:
   :members: before_reset, extract
