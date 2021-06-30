.. _use-reward-functions:

Use Reward Functions
====================

Similarily to :ref:`observation functions <use-observation-functions>` the reward received by
the user for learning can be customized by changing the :py:class:`~ecole.typing.RewardFunction` used by the
solver.
In fact, the mechanism of reward functions is very similar to that of observation
functions: environments do not compute the reward directly but delegate that
responsibility to a :py:class:`~ecole.typing.RewardFunction` object.
The object has complete access to the solver and extracts the data it needs.

Specifying a reward function is performed by passing the :py:class:`~ecole.typing.RewardFunction` object to
the ``reward_function`` environment parameter.
For instance, specifying a reward function with the :py:class:`~ecole.environment.Configuring` environment
looks as follows:

.. doctest::

   >>> env = ecole.environment.Configuring(reward_function=ecole.reward.LpIterations())
   >>> env.reward_function  # doctest: +SKIP
   ecole.reward.LpIterations()
   >>> env.reset("path/to/problem")  # doctest: +ELLIPSIS
   (..., ..., 0.0, ..., ...)
   >>> env.step({})  # doctest: +SKIP
   (..., ..., 45.0, ..., ...)

Environments also have a default reward function, which will be used if the user does not specify any.

.. doctest::

   >>> env = ecole.environment.Configuring()
   >>> env.reward_function  # doctest: +SKIP
   ecole.reward.IsDone()

.. TODO Adapt the output to the actual __repr__ and remove #doctest: +SKIP

See :ref:`the reference<reward-reference>` for the list of available reward functions,
as well as :ref:`the documention<create-new-functions>` for explanations on how to create one.


Arithmetic on Reward Functions
------------------------------
Reinforcement learning in combinatorial optimization solving is an active area of research, and
there is at this point little consensus on reward functions to use. In recognition of that fact,
reward functions have been explicitely designed in Ecole to be easily combined with Python arithmetic.

For instance, one might want to minimize the number of LP iterations used throughout the solving process.
To achieve this using a standard reinforcement learning algorithm, one would might use the negative
number of LP iterations between two steps as a reward: this can be achieved by negating the
:py:class:`~ecole.reward.LpIterations` function.

.. doctest::

   >>> env = ecole.environment.Configuring(reward_function=-ecole.reward.LpIterations())
   >>> env.reset("path/to/problem")  # doctest: +ELLIPSIS
   (..., ..., -0.0, ..., ...)
   >>> env.step({})  # doctest: +SKIP
   (..., ..., -45.0, ..., ...)

More generally, any operation, such as

.. testcode::

   from ecole.reward import LpIterations

   -3.5 * LpIterations() ** 2.1 + 4.4

is valid.

Note that this is a full reward *function* object that can be given to an environment:
it is equivalent to doing the following.

.. doctest::

   >>> env = ecole.environment.Configuring(reward_function=ecole.reward.LpIterations())
   >>> env.reset("path/to/problem")  # doctest: +ELLIPSIS
   (..., ..., ..., ..., ...)
   >>> _, _, lp_iter_reward, _, _ = env.step({})
   >>> reward = -3.5 * lp_iter_reward ** 2.1 + 4.4

Arithmetic operations are even allowed between different reward functions,

.. testcode::

   from ecole.reward import LpIterations, IsDone

   4.0 * LpIterations()**2 - 3 * IsDone()

which is especially powerful because in this normally it would *not* be possible to pass both
:py:class:`~ecole.reward.LpIterations` and :py:class:`~ecole.reward.IsDone` to the
environment.

All operations that are valid between scalars are valid between reward functions.

.. testcode::

   - IsDone() ** abs(LpIterations() // 4)

In addition, not all commonly used mathematical operations have a dedicated Python operator: to
accomodate this, Ecole implements a number of other operations as methods of reward functions.
For instance, to get the exponential of :py:class:`~ecole.reward.LpIterations`, one can use

.. testcode::

   LpIterations().exp()

This also works with rewards functions created from arithmetic expressions.

.. testcode::

   (3 - 2*LpIterations()).exp()

Finally, reward functions have an ``apply`` method to compose rewards with any
function.

.. testcode::

   LpIterations().apply(lambda reward: math.factorial(round(reward)))
