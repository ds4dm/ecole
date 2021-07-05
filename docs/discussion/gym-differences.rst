Differences with OpenAI Gym
===========================

Changing reward and observations
--------------------------------
Contrarily to `OpenAI Gym <https://gym.openai.com/>`_ where learning tasks are predefined,
Ecole gives the user the tools to easily extend and customize environments.
This is because the objective with Ecole is not only to provide a collection of challenges
for machine learning, but really to solve combinatorial optimization problems more
efficiently.
If different data or tweaking the control task delivers better performance, it is an improvement!
This is why Ecole let users change the environment reward and observation using
:py:class:`~ecole.typing.RewardFunction` and :py:class:`~ecole.typing.ObservationFunction`.

Parameter to reset
------------------
In OpenAI Gym, ``reset`` does not take parameters, whereas Ecole
:py:meth:`~ecole.environment.Environment.reset` takes a problem instance as a mandatory
input.
This is because when doing machine learning for optimization, there is no practical interest in
solving the same problem over and over again.
What is important is that the machine learning model is able to generalize to unseen problems.
This is typically done by training on mutliple problem instances.

This setting is similar to multi-task reinforcement learning, where each problem instance is a task
and one aims to generalize to unseen tasks.
An alternative way to implement this is found in `MetaWorld <https://meta-world.github.io/>`_,
where instead of passing the task as a parameter to ``reset``, an supplementary ``set_task`` method
is defined in the environment.

Done on reset
-------------
In Ecole, :py:meth:`~ecole.environment.Environment.reset` returns the same ``done`` flag as
in :py:meth:`~ecole.environment.Environment.step`.
This is because nothing prevents an initial state from also being a terminal one.
It is not only a theoretical consideration: for instance, in :py:class:`~ecole.environment.Branching`,
the initial state would typically be on the root node, prior to making the first branching decision.
However, modern solvers have powerful presolvers, and it is not uncommon that the solution to the
problem is found without needing to branch on any variable.

Action set
----------
Ecole defines an action set at every transition of the environment, while OpenAI Gym defines an
``action_space`` as a static variable of the environment.
Ecole environments are more complex: for instance in :py:class:`~ecole.environment.Branching`
the set of valid actions changes, not only with every episode, but also with every transition!
The ``action_set`` is required to make the next call to
:py:meth:`~ecole.environment.Environment.step`.
We chose to add it as a return type to :py:meth:`~ecole.environment.Environment.step` and
:py:meth:`~ecole.environment.Environment.reset` to emphasize this difference.

Reward offset
-------------
In :py:meth:`~ecole.environment.Environment.reset` a ``reward_offset`` is returned.
This is not only a difference with OpenAI Gym, but also with the MDP formulation.
Its purpose is not to provide additional input to the learning algorithms, but rather to help
researchers better benchmark the resulting performance.
Indeed, :py:class:`~ecole.typing.RewardFunction` are often designed so that their cumulative sum match a
metric on the terminal state, such as solving time or number of LP iterations: this is because final metrics
are often all that matter.
However, for learning, a single reward on the terminal state is hard to learn from.
It is then divided over all intermediate transitions in the episode.

Rather than providing a different mean of evaluating such metrics, we chose to reuse the
environments to compute the cummulative sum, and therfore need the ``reward_offset`` to exactly
match the metric.

No observation on terminal states
---------------------------------
On terminal states, in OpenAI Gym as in Ecole, no further action can be taken and the environment
needs to be :py:meth:`~ecole.environment.Environment.reset`. In Ecole, when an episode is over (that is, when
the ``done`` flag is ``True``), environments always return ``None`` as the observation. This is in contrast with OpenAI Gym,
where some environments do return observations on terminal states.


This can be explained as follows: most of the time, a terminal state in Ecole is a solved problem.
This means that some complex observations cannot be extracted because they require information that
simply does not exist.
For instance, the :py:class:`~ecole.observation.NodeBipartite` observation function extracts some
information about the LP solution of the current branch-and-bound node.
When the problem is solved, for example on a terminal state of the
:py:class:`~ecole.environment.Branching` environment, there might not be a current node, or a linear
relaxation problem, from which this information can be extracted. For these reasons, one would find a
``None`` instead of an observation on terminal states.

In any case, one might note that in reinforcement learning, the observation of a terminal state is usually not very useful.
It is not given to a policy to take the next action (because there are not any), and hence never
used for learning either, so not returning a final observation has no impact in practice.
