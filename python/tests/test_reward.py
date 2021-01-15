"""Test Ecole reward functions in Python.

Most reward functions are written in Ecole C++ library.
This is where the logic should be tested.
The tests provided here run the same assertions on all reward functions.
They mostly test that the code Bindings work as expected.
"""

import math

import pytest

import ecole


def pytest_generate_tests(metafunc):
    """Parametrize the `reward_function` fixture.

    Add reward functions here to have them automatically run all the tests that take
    `reward_function` as input.
    """
    if "reward_function" in metafunc.fixturenames:
        all_reward_functions = (
            ecole.reward.Constant(),
            ecole.reward.IsDone(),
            ecole.reward.LpIterations(),
        )
        metafunc.parametrize("reward_function", all_reward_functions)


def advance_to_root_node(model):
    """Utility to advance a model to the root node."""
    dyn = ecole.dynamics.BranchingDynamics()
    dyn.reset_dynamics(model)
    return model


def test_default_init(reward_function):
    """Construct with default arguments."""
    type(reward_function)()


def test_before_reset(reward_function, model):
    """Successive calls to before_reset."""
    reward_function.before_reset(model)
    reward_function.before_reset(model)


@pytest.mark.parametrize("done", [True, False])
def test_extract(reward_function, done, model):
    """Rewards are floats."""
    reward_function.before_reset(model)
    advance_to_root_node(model)
    reward = reward_function.extract(model, done)
    assert isinstance(reward, float)


@pytest.mark.parametrize("done", [True, False])
def test_reproducability(reward_function, done, model, model_copy):
    """Same trajectories yield same rewards."""
    reward_function.before_reset(model)
    advance_to_root_node(model)
    reward1 = reward_function.extract(model, done)

    reward_function.before_reset(model_copy)
    advance_to_root_node(model_copy)
    reward2 = reward_function.extract(model_copy, done=done)

    assert reward1 == reward2


@pytest.mark.parametrize(
    "func_formula,reward_formula",
    [
        [lambda r: -r] * 2,
        [lambda r: r - 3] * 2,
        [lambda r: 3 - r] * 2,
        [lambda r: abs(-r) + 2] * 2,
        [lambda rf: rf.exp(), lambda r: math.exp(r)],
        [lambda rf: rf.apply(lambda r: r + 2), lambda r: r + 2],
    ],
)
def test_operators(reward_function, model, model_copy, func_formula, reward_formula):
    """Operators produce operations on rewards."""
    reward_function.before_reset(model)
    advance_to_root_node(model)
    reward = reward_function.extract(model)
    # WARNING reward_function and formula_reward_function share underlying reference with current
    # Python implementation but the test works due to reward function reproducability.
    formula_reward_function = func_formula(reward_function)
    formula_reward_function.before_reset(model_copy)
    advance_to_root_node(model_copy)
    formula_reward = formula_reward_function.extract(model_copy)
    assert formula_reward == reward_formula(reward)


def test_cumsum(reward_function, model, model_copy):
    """Operators produce operations on rewards."""
    reward_function.before_reset(model)
    advance_to_root_node(model)
    reward1 = reward_function.extract(model)
    reward2 = reward_function.extract(model)
    # WARNING reward_function and cum_reward_function share underlying reference with current
    # Python implementation but the test works due to reward function reproducability.
    cum_reward_function = reward_function.cumsum()
    cum_reward_function.before_reset(model_copy)
    advance_to_root_node(model_copy)
    cum_reward1 = cum_reward_function.extract(model_copy)
    cum_reward2 = cum_reward_function.extract(model_copy)

    assert cum_reward1 == reward1
    assert cum_reward2 == reward1 + reward2
