Rewards
=======

Protocol
--------
The protocol expected to define a valid reward function is given below.

.. autoclass:: ecole.typing.RewardFunction
   :members:

Listing
-------
The list of reward functions relevant to users is given below.

Is Done
^^^^^^^
.. autoclass:: ecole.reward.IsDone
   :members: reset, obtain_reward

LP Iterations
^^^^^^^^^^^^^
.. autoclass:: ecole.reward.LpIterations
   :members: reset, obtain_reward


Utilities
---------
The following reward functions are used internally by Ecole.

Constant
^^^^^^^^
.. autoclass:: ecole.reward.Constant
   :members: reset, obtain_reward

Arithmetic
^^^^^^^^^^
.. autoclass:: ecole.reward.Arithmetic
   :members: reset, obtain_reward
