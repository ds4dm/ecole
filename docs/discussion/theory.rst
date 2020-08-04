Ecole Theoretical Model
=======================

The Ecole API and classes directly relate to the different components of
an episodic `partially-observable Markov decision process <https://en.wikipedia.org/wiki/Partially_observable_Markov_decision_process>`_
(PO-MDP).

Markov decision process
-----------------------
Consider a regular Markov decision process
:math:`(\mathcal{S}, \mathcal{A}, p_\textit{init}, p_\textit{trans}, R)`, whose components are

* a state space :math:`\mathcal{S}`
* an action space :math:`\mathcal{A}`
* an initial state distribution :math:`p_\textit{init}: \mathcal{S} \to \mathbb{R}_{\geq 0}`
* a state transition distribution
  :math:`p_\textit{trans}: \mathcal{S} \times \mathcal{A} \times \mathcal{S} \to \mathbb{R}_{\geq 0}`
* a reward function :math:`R: \mathcal{S} \to \mathbb{R}`.

.. note::

    The choice of having deterministic rewards :math:`r_t = R(s_t)` is
    arbitrary here, in order to best fit the Ecole API. Note that it is
    not a restrictive choice though, as any MDP with stochastic rewards
    :math:`r_t \sim p_\textit{reward}(r_t|s_{t-1},a_{t-1},s_{t})`
    can be converted into an equivalent MDP with deterministic ones,
    by considering the reward as part of the state.

Together with an action policy

.. math::

    \pi: \mathcal{A} \times \mathcal{S} \to \mathbb{R}_{\geq 0}

an MDP can be unrolled to produce state-action trajectories

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
    that correspond to continuing tasks. In Ecole we garantee that all
    environments correspond to **episodic** tasks, that is, each episode is
    garanteed to start from an initial state :math:`s_0`, and end in a
    terminal state :math:`s_\textit{final}`. For convenience this terminal state can
    be considered as absorbing, i.e.,
    :math:`p_\textit{trans}(s_{t+1}|a_t,s_t=s_\textit{final}) := \delta_{s_\textit{final}}(s_{t+1})`,
    and associated to a null reward, :math:`R(s_\textit{final}) := 0`, so that all
    future states encountered after :math:`s_\textit{final}` can be safely ignored in
    the MDP control problem.

Partially-observable Markov decision process
--------------------------------------------
In the PO-MDP setting, complete information about the current MDP state
is not necessarily available to the decision-maker. Instead,
at each step only a partial observation :math:`o \in \Omega`
is made available, which can be seen as the result of applying an observation
function :math:`O: \mathcal{S} \to \Omega` to the current state. As a result,
PO-MDP trajectories take the form

.. math::

   \tau=(o_0,r_0,a_0,o_1\dots)
   \text{,}

where :math:`o_t:= O(s_t)` and :math:`r_t:=R(s_t)` are respectively the
observation and the reward collected at time step :math:`t`. Due to the
non-Markovian nature of those trajectories, that is,

.. math::

    o_{t+1},r_{t+1} \mathop{\rlap{\perp}\mkern2mu{\not\perp}} o_0,r_0,a_0,\dots,o_{t-1},r_{t-1},a_{t-1} \mid o_t,r_t,a_t
    \text{,}

the decision-maker must take into account the whole history of past
observations, rewards and actions, in order to decide on an optimal action
at current time step :math:`t`. The PO-MDP policy then takes the form

.. math::

   \pi:\mathcal{A} \times \mathcal{H} \to \mathbb{R}_{\geq 0}
   \text{,}

where :math:`h_t:=(o_0,r_0,a_0,\dots,o_t,r_t)\in\mathcal{H}` represents the
PO-MDP history at time step :math:`t`, so that :math:`a_t \sim \pi(a_t|h_t)`.

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

The following Ecole components can be directly translated into PO-MDP
components from the above formulation:

* :py:class:`~ecole.typing.RewardFunction` <=> :math:`R`
* :py:class:`~ecole.typing.ObservationFunction` <=> :math:`O`
* :py:meth:`~ecole.typing.Dynamics.reset_dynamics` <=> :math:`p_\textit{init}(s_0)`
* :py:meth:`~ecole.typing.Dynamics.step_dynamics` <=> :math:`p_\textit{trans}(s_{t+1}|s_t,a_t)`

The :py:class:`~ecole.environment.EnvironmentComposer` class wraps all of
those components together to form the PO-MDP. Its API can be interpreted as
follows:

* :py:meth:`~ecole.environment.EnvironmentComposer.reset` <=>
  :math:`s_0 \sim p_\textit{init}(s_0), r_0=R(s_0), o_0=O(s_0)`
* :py:meth:`~ecole.environment.EnvironmentComposer.step` <=>
  :math:`s_{t+1} \sim p_\textit{trans}(s_{t+1}|a_s,s_t), r_t=R(s_t), o_t=O(s_t)`
* ``done == True`` <=> the PO-MDP will now enter the terminal state,
  :math:`s_{t+1}==s_\textit{final}`. As such, the current episode ends now.

The state space :math:`\mathcal{S}` can be considered to be the whole computer
memory occupied by the environment, which includes the state of the underlying
SCIP solver instance. The action space :math:`\mathcal{A}` is specific to each
environment.

.. note::
   We allow the environment to specify a set of valid actions at each time
   step :math:`t`. The ``action_set`` value returned by
   :py:meth:`~ecole.environment.EnvironmentComposer.reset` and
   :py:meth:`~ecole.environment.EnvironmentComposer.step` serves this purpose,
   and can be left to ``None`` when the action set is implicit.


.. note::

   As can be seen from :eq:`pomdp_control`, the initial reward :math:`r_0`
   returned by :py:meth:`~ecole.environment.EnvironmentComposer.reset`
   does not affect the control problem. In Ecole we
   nevertheless chose to preserve this initial reward, in order to obtain
   meaningfull cumulated episode rewards (e.g., total running time).
