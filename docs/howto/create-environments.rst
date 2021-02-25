.. _create-new-environment:

Create New Environments
=======================

Environment Structure
---------------------
In Ecole, it is possible to customize the :ref:`reward<use-reward-functions>` or
:ref:`observation<use-reward-functions>` returned by the environment.
The :py:class:`~ecole.typing.RewardFunction` and :py:class:`~ecole.typing.ObservatioFunction` are
taking some responsability away from the environment.
We call what is left, *i.e.* an environment without rewards or observations, the environment
:py:class:`~ecole.typing.Dynamics`.
In other words, the dynamics define the bare bone transitions of the Markov Decision Process.

Dynamics have an interface similar to environments, but with different input parameters and return
types.
In fact environments are wrapper around dynamics that drive the following orchestration:

* Environments store the state as a :py:class:`~ecole.scip.Model`;
* Forward the :py:class:`~ecole.scip.Model` to the :py:class:`~ecole.typing.Dynamics` to start a new
  episode or transition to receive an action set;
* Forward the :py:class:`~ecole.scip.Model` to the :py:class:`~ecole.typing.RewardFunction` and
  :py:class:`~ecole.typing.ObservationFunction` to recieve an observation and reward;
* Return everything to the user.

One susbtancial difference between the environment and the dynamics is the seeding behaviour.
Given that this is not an easy topic, it is discussed in :ref:`seeding-discussion`.

Creating Dynamics
-----------------
Reset and Step
^^^^^^^^^^^^^^
Creating dynamics is very similar to
:ref:`creating reward and observation functions<create-new-functions>`.
It can be done from scratch or by inheriting an existing one.
The following examples shows how we can inherit :py:class:`~ecole.dynamics.BranchingDynamics` to
deactivate cutting plane and presolve.

.. note::

   For directly changing SCIP parameters, directly pass them to the
   :ref:`environment construtor<environment-parameters>`.

Given that there is a large number of parameters to change, we want to use one of SCIP default mode
by calling ``SCIPsetPresolving`` and ``SCIPsetSeparating`` through PyScipOpt
(`SCIP doc <https://www.scipopt.org/doc/html/group__ParameterMethods.php>`_).

We will do so by overriding :py:meth:`~ecole.dynamics.BranchingDynamics.reset_dynamics`, which
get called by :py:meth:`~ecole.environment.Environment.reset`.
The similar method :py:meth:`~ecole.dynamics.BranchingDynamics.step_dynamics`, which is called
by :py:meth:`~ecole.environment.Environment.step` does not need to be changed in this
example so we do not override it.

.. testcode::
   :skipif: pyscipopt is None

   import ecole
   from pyscipopt.scip import PY_SCIP_PARAMSETTING


   class SimpleBranchingDynamics(ecole.dynamics.BranchingDynamics):

       def reset_dynamics(self, model):
           # Share memory with Ecole model
           pyscipopt_model = model.as_pyscipopt()

           pyscipopt_model.setPresolve(PY_SCIP_PARAMSETTING.OFF)
           pyscipopt_model.setSeparating(PY_SCIP_PARAMSETTING.OFF)

           # Let the parent class get the model to the root node and return
           # the done flag / action_set
           return super().reset_dynamics(model)


With the ``SimpleBranchingDynamics``, we have defined what we want the solver to do.
Now, to use it as a full environent that can manage observations and rewards, we wrap it with the
:py:class:`~ecole.environment.Environment`.


.. testcode::
   :skipif: pyscipopt is None

   class SimpleBranching(ecole.environment.Environment):
       __Dynamics__ = SimpleBranchingDynamics


``SimpleBranching`` is a fully featured environment as any other in Ecole.

Passing parameters
^^^^^^^^^^^^^^^^^^
We can make the previous example more flexible by deciding what we want to disable.
To do so, we will take parameters in the constructor

.. testcode::
   :skipif: pyscipopt is None

   class SimpleBranchingDynamics(ecole.dynamics.BranchingDynamics):

       def __init__(self, disable_presolve=True, disable_cuts=True, *args, **kwargs):
           super().__init__(*args, **kwargs)
           self.disable_presolve = disable_presolve
           self.disable_cuts = disable_cuts

       def reset_dynamics(self, model):
           # Share memory with Ecole model
           pyscipopt_model = model.as_pyscipopt()

           if self.disable_presolve:
               pyscipopt_model.setPresolve(PY_SCIP_PARAMSETTING.OFF)
           if self.disable_cuts:
               pyscipopt_model.setSeparating(PY_SCIP_PARAMSETTING.OFF)

           # Let the parent class get the model to the root node and return
           # the done flag / action_set
           return super().reset_dynamics(model)


   class SimpleBranching(ecole.environment.Environment):
       __Dynamics__ = SimpleBranchingDynamics


The constructor arguments are forwarded from the :py:meth:`~ecole.environment.EnvironmentComposer.__init__` constructor:

.. testcode::
   :skipif: pyscipopt is None

   env = SimpleBranching(observation_function=None, disable_cuts=False)

Similarily, extra arguments given to the environemnt :py:meth:`~ecole.environment.EnvironmentComposer.reset` and
:py:meth:`~ecole.environment.EnvironmentComposer.step` are forwarded to the associated
:py:class:`~ecole.typing.Dynamics` methods.
