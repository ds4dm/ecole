.. _theory:

Ecole Theoretical Model
=======================

The Ecole API directly relates to the different components of
an episodic `partially-observable Markov decision process <https://en.wikipedia.org/wiki/Partially_observable_Markov_decision_process>`_
(PO-MDP).

Markov decision process
-----------------------
Consider a regular Markov decision process
:math:`(\mathcal{S}, \mathcal{A}, p_\textit{init}, p_\textit{trans}, R)`,
whose components are

* a state space :math:`\mathcal{S}`
* an action space :math:`\mathcal{A}`
* an initial state distribution :math:`p_\textit{init}: \mathcal{S} \to \mathbb{R}_{\geq 0}`
* a state transition distribution
  :math:`p_\textit{trans}: \mathcal{S} \times \mathcal{A} \times \mathcal{S} \to \mathbb{R}_{\geq 0}`
* a reward function :math:`R: \mathcal{S} \to \mathbb{R}`.

.. note::

    Having deterministic rewards :math:`r_t = R(s_t)` is an arbitrary choice
    here, in order to best fit the Ecole API. It is not restrictive though,
    as any MDP with stochastic rewards
    :math:`r_t \sim p_\textit{reward}(r_t|s_{t-1},a_{t-1},s_{t})`
    can be converted into an equivalent MDP with deterministic ones,
    by considering the reward as part of the state.

Together with an action policy

.. math::

    \pi: \mathcal{A} \times \mathcal{S} \to \mathbb{R}_{\geq 0}

such that :math:`a_t \sim \pi(a_t|s_t)`, an MDP can be unrolled to produce
state-action trajectories

.. math::

   \tau=(s_0,a_0,s_1,\dots)

that obey the following joint distribution

.. math::

    \tau \sim \underbrace{p_\textit{init}(s_0)}_{\text{initial state}}
    \prod_{t=0}^\infty \underbrace{\pi(a_t | s_t)}_{\text{next action}}
    \underbrace{p_\textit{trans}(s_{t+1} | a_t, s_t)}_{\text{next state}}
    \text{.}

MDP control problem
^^^^^^^^^^^^^^^^^^^
We define the MDP control problem as that of finding a policy
:math:`\pi^\star` which is optimal with respect to the expected total
reward,

.. math::
   :label: mdp_control

   \pi^\star = \underset{\pi}{\operatorname{arg\,max}}
   \lim_{T \to \infty} \mathbb{E}_\tau\left[\sum_{t=0}^{T} r_t\right]
   \text{,}

where :math:`r_t := R(s_t)`.

.. note::

    In the general case this quantity may not be bounded, for example for MDPs
    corresponding to *continuing* tasks where episode length may be infinite.
    In Ecole, we guarantee that all environments correspond to *episodic*
    tasks, that is, each episode is guaranteed to end in a terminal state.
    This can be modeled by introducing a null state :math:`s_\textit{null}`,
    such that

    * :math:`s_\textit{null}` is absorbing, i.e., :math:`p_\textit{trans}(s_{t+1}|a_t,s_t=s_\textit{null}) := \delta_{s_\textit{null}}(s_{t+1})`
    * :math:`s_\textit{null}` yields no reward, i.e., :math:`R(s_\textit{null}) := 0`
    * a state :math:`s` is terminal :math:`\iff` it transitions
      into the null state with probability one, i.e., :math:`p_\textit{trans}(s_{t+1}|a_t,s_t=s) := \delta_{s_\textit{null}}(s_{t+1})`

    As such, all actions and states encountered after a terminal state
    can be safely ignored in the MDP control problem.

Partially-observable Markov decision process
--------------------------------------------
In the PO-MDP setting, complete information about the current MDP state
is not necessarily available to the decision-maker. Instead,
at each step only a partial observation :math:`o \in \Omega`
is made available, which can be seen as the result of applying an observation
function :math:`O: \mathcal{S} \to \Omega` to the current state. As such, a
PO-MDP consists of a tuple
:math:`(\mathcal{S}, \mathcal{A}, p_\textit{init}, p_\textit{trans}, R, O)`.

.. note::

    Similarly to having deterministic rewards, having deterministic
    observations is an arbitrary choice here, but is not restrictive.

As a result, PO-MDP trajectories take the form

.. math::

   \tau=(o_0,r_0,a_0,o_1\dots)
   \text{,}

where :math:`o_t:= O(s_t)` and :math:`r_t:=R(s_t)` are respectively the
observation and the reward collected at time step :math:`t`.

Let us now introduce a convenience variable
:math:`h_t:=(o_0,r_0,a_0,\dots,o_t,r_t)\in\mathcal{H}` that represents the
PO-MDP history at time step :math:`t`. Due to the non-Markovian nature of
the trajectories, that is,

.. math::

    o_{t+1},r_{t+1} \mathop{\rlap{\perp}\mkern2mu{\not\perp}} h_{t-1} \mid o_t,r_t,a_t
    \text{,}

the decision-maker must take into account the whole history of observations,
rewards and actions in order to decide on an optimal action at current time
step :math:`t`. PO-MDP policies then take the form

.. math::

   \pi:\mathcal{A} \times \mathcal{H} \to \mathbb{R}_{\geq 0}

such that :math:`a_t \sim \pi(a_t|h_t)`.

PO-MDP control problem
^^^^^^^^^^^^^^^^^^^^^^
The PO-MDP control problem can then be written identically to the MDP one,

.. math::
   :label: pomdp_control

   \pi^\star = \underset{\pi}{\operatorname{arg\,max}} \lim_{T \to \infty}
   \mathbb{E}_\tau\left[\sum_{t=0}^{T} r_t\right]
   \text{.}

Ecole as PO-MDP components
--------------------------

The following Ecole components directly translate into PO-MDP components from
the aforementioned formulation:

* :py:class:`~ecole.typing.RewardFunction` <=> :math:`R`
* :py:class:`~ecole.typing.ObservationFunction` <=> :math:`O`
* :py:meth:`~ecole.typing.Dynamics.reset_dynamics` <=>
  :math:`p_\textit{init}(s_0)`
* :py:meth:`~ecole.typing.Dynamics.step_dynamics` <=>
  :math:`p_\textit{trans}(s_{t+1}|s_t,a_t)`

The state space :math:`\mathcal{S}` can be considered to be the whole computer
memory occupied by the environment, which includes the state of the underlying
SCIP solver instance. The action space :math:`\mathcal{A}` is specific to each
environment.

.. note::

   In practice, both :py:class:`~ecole.typing.RewardFunction` and
   :py:class:`~ecole.typing.ObservationFunction` are implemented as stateful
   classes, and therefore should be considered as part of the MDP state
   :math:`s`. This *extended* state is not meant to take part in the MDP
   dynamics per se, but nonetheless it has to be considered as the actual
   PO-MDP state, in order to allow for a strict interpretation of Ecole
   environments as PO-MDPs.

The :py:class:`~ecole.environment.Environment` class wraps all of
those components together to form the actual PO-MDP. Its API can be
interpreted as follows:

* :py:meth:`~ecole.environment.Environment.reset` <=>
  :math:`s_0 \sim p_\textit{init}(s_0), r_0=R(s_0), o_0=O(s_0)`
* :py:meth:`~ecole.environment.Environment.step` <=>
  :math:`s_{t+1} \sim p_\textit{trans}(s_{t+1}|a_t,s_t), r_t=R(s_t), o_t=O(s_t)`
* ``done == True`` <=> the current state :math:`s_{t}` is terminal. As such,
  the episode ends now.

.. note::

   In Ecole we allow environments to optionally specify a set of valid
   actions at each time step :math:`t`. To this end, both the
   :py:meth:`~ecole.environment.Environment.reset` and
   :py:meth:`~ecole.environment.Environment.step` methods return
   the valid ``action_set`` for the next transition, in addition to the
   current observation and reward. This action set is optional, and
   environments in which the action set is implicit may simply return
   ``action_set == None``.

Implementation of both the PO-MDP policy :math:`\pi(a_t|h_t)` and a method
to solve the resulting control problem :eq:`pomdp_control` is left to the
user.

.. note::

   As can be seen from :eq:`mdp_control` and :eq:`pomdp_control`, the initial
   reward :math:`r_0` returned by
   :py:meth:`~ecole.environment.Environment.reset`
   does not affect the control problem. In Ecole we
   nevertheless chose to preserve this initial reward, in order to obtain
   meaningfull cumulated episode rewards, such as the total running time
   (which must include the time spend in
   :py:meth:`~ecole.environment.Environment.reset`), or the total
   number of branch-and-bound nodes in a
   :py:class:`~ecole.environment.Branching` environment (which must include
   the root node).
