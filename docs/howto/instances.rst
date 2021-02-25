.. _generate-instances:

Generate Problem Instances
==========================

Ecole contains a number of problem :py:class:`~ecole.typing.InstanceGenerator` in the ``ecole.instance`` module.
They generate instances as :py:class:`ecole.scip.Model`.
To generate instances, first instantiate a generator.
All generators are constructed with different parameters depending on the problem type.
An :py:class:`~ecole.typing.InstanceGenerator` is infinite `Python iterators <https://wiki.python.org/moin/Iterator>`_ so
we can iterate over them using any of Python iterating mechnisms.

For instance, to generate `set covering problems <https://en.wikipedia.org/wiki/Set_cover_problem>`_, one would use
:py:class:`~ecole.instance.SetCoverGenerator` in the following fashion:

.. testcode::

   from ecole.instance import SetCoverGenerator


   generator = SetCoverGenerator(n_rows=100, n_cols=200, density=0.1)

   for i in range(50):
       instance = next(generator)

       # Do anything with the ecole.scip.Model
       instance.write_problem("some-folder/set-cover-{i:04}.lp")


Note how we are iterating over a ``range(50)`` and calling ``next`` on the generator.
This is because iterating directly over the iterator would produce an infinte loop.
For users more comfortable with iterators, other possibilities exists, such as
`islice <https://docs.python.org/3/library/itertools.html#itertools.islice>`_.


Generators Random States
------------------------
An :py:class:`~ecole.typing.InstanceGenerator` holds a random state to generate instance.
This can be better understood when using the :py:meth:`~ecole.typing.InstanceGenerator.seed` method of the generator.

.. testcode::

   generator_a = SetCoverGenerator(n_rows=100, n_cols=200, density=0.1)
   generator_b = SetCoverGenerator(n_rows=100, n_cols=200, density=0.1)

   # These are not the same instance
   instance_a = next(generator_a)
   instance_b = next(generator_b)

   generator_a.seed(809)
   generator_b.seed(809)

   # These are exactly the same instances
   instance_a = next(generator_a)
   instance_b = next(generator_b)


With an Environment
-------------------
The environment :py:meth:`~ecole.environment.EnvironmentComposer.reset` accepts problem instance as
:py:class:`ecole.scip.Model`, so there is no need to write generated instances to file.

A typical example training over 1000 instances/episodes would look like:

.. testcode::

   import ecole


   env = ecole.environment.Branching()
   gen = ecole.instance.SetCoverGenerator(n_rows=100, n_cols=200)

   for _ in range(1000):
       observation, action_set, reward_offset, done, info = env.reset(next(gen))
       while not done:
           observation, action_set, reward, done, info = env.step(action_set[0])

.. note::
   While it is possible to modify the instance before passing it to
   :py:meth:`~ecole.environment.EnvironmentComposer.reset`, it is not considered a good practice, as it obscure what
   what task is being learned (which is not self contained by the environment class anymore).
   A better alternative is to :ref:`create a new environment<create-new-environment>` to perfom such changes.


Adapt Instance Generators
-------------------------
An :py:class:`~ecole.typing.InstanceGenerator` only create instances for users to consume.
Therefore, there is no constraints on how iterating over instance should be done, it is entirely up to the user.
Using different data structure, such as lists, dictionaries, *etc.* is completely valid because environments never
"*see*" generators, only the instances.
Here we illustrate some possibilities to adapt Ecole instance generators.
Python's ``yield`` keyword can make it very compact to create iterators.

Combine Multiple Generators
^^^^^^^^^^^^^^^^^^^^^^^^^^^
To learn over multiple problem types, one could build a generator that, for every instance to generate, chooses a
a problem type at random, and returns it.

.. testcode::

   import random


   def CombineGenerators(*generators):
       # A random state for choice
       random_engine = random.Random()
       while True:
           # Randomly pick a generator
           gen = random_engine.choice(generators)
           # And yield the instance it generates
           yield next(gen)


This generator does not have a ``seed`` method.
If we want to implement it, we have to write the same generator as the equilvalent class.

.. testcode::

   class CombineGenerators:
       def __init__(self, *generators):
           self.generators = generators
           self.random_engine = random.Random()

       def __next__(self):
           return next(self.random_engine.choice(self.generators))

       def __iter__(self):
           return self

       def seed(self, val):
           self.random_engine.seed(val)
           for gen in self.generators:
               gen.seed(val)

Generator Random Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^
Another useful case it to generate instances of a same problem type but with different parameters.
If there are few different set of parameter to choose from, then we could use the same technique as above.
However, with more set of parameters (or even infinite), this becomes wasteful (or impossible).

To do this, we can use the generator's :py:meth:`~ecole.typing.InstanceGenerator.generate_instance` static function
and manually pass a :py:class:`~ecole.RandomEngine`.
For instance, to randomly choose the ``n_cols`` and ``n_rows`` parameters from
:py:class:`~ecole.instance.SetCoverGenerator`, one could use

.. testcode::

   import random
   import ecole


   class VariableSizeSetCoverGenerator:
       def __init__(self, n_cols_range, n_rows_range):
           self.n_cols_range = n_cols_range
           self.n_rows_range = n_rows_range
           # A Python radnom state for randint
           self.py_random_engine = random.Random()
           # An Ecole random state to pass to generating functions.
           # This function returns a random state whose seed depends on Ecole global random state
           self.ecole_random_engine = ecole.spawn_random_engine()

       def __next__(self):
           return ecole.instance.SetCoverGenerator(
               n_cols=self.py_random_engine.randint(*self.n_cols_range),
               n_rows=self.py_random_engine.randint(*self.n_rows_range),
               random_engine=self.ecole_random_engine,
           )

       def __iter__(self):
           return self

       def seed(self, val):
           self.py_random_engine.seed(val)
           self.ecole_random_engine.seed(val)


See :ref:`the discussion on seeding<seeding-discussion>` for an explanation of :py:func:`ecole.spawn_random_engine`.
