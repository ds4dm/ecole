.. _create-new-environment:

Create New Environments
=======================

Environment Structure
---------------------
In Ecole, it is possible to customize the :ref:`reward<use-reward-functions>` or
:ref:`observation<use-reward-functions>` returned by the environment. These components are structured in
:py:class:`~ecole.typing.RewardFunction` and :py:class:`~ecole.typing.ObservationFunction` classes that are
independent from the rest of the environment. We call what is left, that is, the environment without rewards
or observations, the environment's :py:class:`~ecole.typing.Dynamics`.
In other words, the dynamics define the bare bone transitions of the Markov Decision Process.

Dynamics have an interface similar to environments, but with different input parameters and return
types.
In fact environments are wrappers around dynamics classes that drive the following orchestration:

* Environments store the state as a :py:class:`~ecole.scip.Model`;
* Then, they forward the :py:class:`~ecole.scip.Model` to the :py:class:`~ecole.typing.Dynamics` to start a new
  episode or transition to receive an action set;
* Next, they forward the :py:class:`~ecole.scip.Model` to the :py:class:`~ecole.typing.RewardFunction` and
  :py:class:`~ecole.typing.ObservationFunction` to receive an observation and reward;
* Finally, return everything to the user.

One susbtantial difference between the environment and the dynamics is the seeding behavior.
Given that this is not an easy topic, it is discussed in :ref:`seeding-discussion`.

Creating Dynamics
-----------------

Reset and Step
^^^^^^^^^^^^^^
Creating dynamics is very similar to :ref:`creating reward and observation functions<create-new-functions>`.
It can be done from scratch or by inheriting an existing one.
The following examples show how we can inherit a :py:class:`~ecole.dynamics.BranchingDynamics` class to
deactivate cutting planes and presolving in SCIP.

.. note::

   One can also more directly deactivate SCIP parameters through the
   :ref:`environment constructor<environment-parameters>`.

Given that there is a large number of parameters to change, we want to use one of SCIP default's modes
by calling ``SCIPsetPresolving`` and ``SCIPsetSeparating`` through PyScipOpt
(`SCIP doc <https://www.scipopt.org/doc/html/group__ParameterMethods.php>`_).

We will do so by overriding :py:meth:`~ecole.dynamics.BranchingDynamics.reset_dynamics`, which
gets called by :py:meth:`~ecole.environment.Environment.reset`.
The similar method :py:meth:`~ecole.dynamics.BranchingDynamics.step_dynamics`, which is called
by :py:meth:`~ecole.environment.Environment.step`, does not need to be changed in this
example, so we do not override it.

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

           # Let the parent class get the model at the root node and return
           # the done flag / action_set
           return super().reset_dynamics(model)


With our ``SimpleBranchingDynamics`` class we have defined what we want the solver to do.
Now, to use it as a full environment that can manage observations and rewards, we wrap it in an
:py:class:`~ecole.environment.Environment`.


.. testcode::
   :skipif: pyscipopt is None

   class SimpleBranching(ecole.environment.Environment):
       __Dynamics__ = SimpleBranchingDynamics


The resulting ``SimpleBranching`` class is then an environment as valid as any other in Ecole.

Passing parameters
^^^^^^^^^^^^^^^^^^
We can make the previous example more flexible by deciding what we want to disable.
To do so, we will take parameters in the constructor.

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

           # Let the parent class get the model at the root node and return
           # the done flag / action_set
           return super().reset_dynamics(model)


   class SimpleBranching(ecole.environment.Environment):
       __Dynamics__ = SimpleBranchingDynamics


The constructor arguments are forwarded from the :py:meth:`~ecole.environment.Environment.__init__` constructor:

.. testcode::
   :skipif: pyscipopt is None

   env = SimpleBranching(observation_function=None, disable_cuts=False)

Similarily, extra arguments given to the environemnt :py:meth:`~ecole.environment.Environment.reset` and
:py:meth:`~ecole.environment.Environment.step` are forwarded to the associated
:py:class:`~ecole.typing.Dynamics` methods.

Using Control Inversion
-----------------------
When using a traditional SCIP callback, the user has to add the callback to SCIP, call ``SCIPsolve``, and wait for the
solving process to terminate.
We say that *SCIP has the control*.
This has some downsides, such a having to forward all the data the agent will use to the callback, making it harder to
stop the solving process, and reduce interactivity.
For instance when using a callback in a notebook, if the user forgot to fetch some data, then they have to re-execute
the whole solving process.

On the contrary, when using an Ecole environment such as :py:class:`~ecole.environment.Branching`, the environment
pauses on every branch-and-bound node (*i.e.* every branchrule callback call) to let the user make a decision,
or inspect the :py:class:`~ecole.scip.Model`.
We say that the *user (or the agent) has the control*.
To do so, we did not reconstruct the solving algorithm ``SCIPsolve`` to fit our needs.
Rather, we have implemented a general *inversion of control* mechanism to let SCIP pause and be resumed on every
callback call (using a form of *stackful coroutine*).
We call this approach *iterative solving* and it runs exactly the same ``SCIPsolve`` algorithm, without noticable
overhead, while perfectly forwarding all information available in the callback.

To use this tool, the user start by calling :py:meth:`ecole.scip.Model.solve_iter`, with a set of call callback
constructor arguments.
Iterative solving will then add these callbacks, start solving, and return the first time that one of these callback
is executed.
The return value describes where the solving has stopped, and the parameters of the callback where it has stopped.
This is the time for the user to perform whichever action they would have done in the callback.
Solving can be resumed by calling :py:meth:`ecole.scip.Model.solve_iter_continue` with the
:py:class:`ecole.scip.callback.Result` that would have been set in the callback.
Solving is finished when one of the iterative solving function returns ``None``.
The :py:class:`ecole.scip.Model` can safely be deleted an any time (SCIP termination is handled automatically).

For instance, iterative solving an environement while pausing on branchrule and heuristic callbacks look like the
following.

.. testcode::

   model = ecole.scip.Model.from_file("path/to/file")

   # Start solving until the first pause, if any.
   fcall = model.solve_iter(
       # Stop on branchrule callback.
       ecole.scip.callback.BranchruleConstructor(),
       # Stop on heuristic callback after node.
       ecole.scip.callback.HeuristicConstructor(timing_mask=ecole.scip.HeurTiming.AfterNode),
   )
   # While solving is not finished, `fcall` contains information about the current stop.
   while fcall is not None:
       # Solving stopped on a branchrule callback.
       if isinstance(fcall, ecole.scip.callback.BranchruleCall):
           # Perform some branching (through PyScipOpt).
           ...
           # Resume solving until next pause.
           fcall = model.solve_iter_continue(ecole.scip.callback.Result.Branched)
       # Solving stopped on a heurisitc callback.
       elif isinstance(fcall, ecole.scip.callback.HeuristicCall):
           # Return as no heuristic was performed (only data collection)
           fcall = model.solve_iter_continue(ecole.scip.callback.Result.DidNotRun)

See :py:class:`~ecole.scip.callback.BranchruleConstructor`, :py:class:`~ecole.scip.callback.HeuristicConstructor` for
callback constructor parameters, as well as :py:class:`~ecole.scip.callback.BranchruleCall` and
:py:class:`~ecole.scip.callback.BranchruleCall` for callbacks functions parameters passed by SCIP to the callback
methods.

.. note::

   By default callback parameters such as ``priority``, ``frequency``, and ``max_depth`` taht control how when
   the callback are evaluated by SCIP are set to run as often as possible.
   However, it is entirely possible to run it with lower priority or frequency for create specific environments or
   whatever other purpose.

To create dynamics using iterative solving, one should call :py:meth:`ecole.scip.Model.solve_iter` in
:py:meth:`~ecole.typing.Dynamics.reset_dynamics` and :py:meth:`ecole.scip.Model.solve_iter_continue` in
:py:meth:`~ecole.typing.Dynamics.step_dynamics`.
For instance, a branching environment could be created with the following dynamics.

.. testcode::
   :skipif: pyscipopt is None

   class MyBranchingDynamics:
       def __init__(self, pseudo_candidates=False, max_depth=ecole.scip.callback.max_depth_none):
           self.pseudo_candidates = pseudo_candidates
           self.max_depth = max_depth

       def action_set(self, model):
           if self.pseudo_candidates:
               return model.as_pyscipopt().getPseudoBranchCands()
           else:
               return model.as_pyscipopt().getLPBranchCands()
           return ...

       def reset_dynamics(self, model):
           fcall = model.solve_iter(
               ecole.scip.callback.BranchruleConstructor(max_depth=self.max_depth)
           )
           return (fcall is None), self.action_set(model)

       def step_dynamics(self, model, action):
           model.as_pyscipopt().branchVar(action)
           fcall = model.solve_iter_continue(ecole.scip.callback.Result.Branched)
           return (fcall is None), self.action_set(model)
