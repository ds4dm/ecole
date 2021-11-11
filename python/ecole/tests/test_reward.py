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
            ecole.reward.NNodes(),
            ecole.reward.LpIterations(),
            ecole.reward.SolvingTime(),
            ecole.reward.PrimalIntegral(bound_function=lambda x: (0.0, 0.0)),
            ecole.reward.DualIntegral(bound_function=lambda x: (0.0, 0.0)),
            ecole.reward.PrimalDualIntegral(bound_function=lambda x: (0.0, 0.0)),
        )
        metafunc.parametrize("reward_function", all_reward_functions)


def test_default_init(reward_function):
    """Construct with default arguments."""
    type(reward_function)()


def test_before_reset(reward_function, model, model_copy):
    """Successive calls to before_reset."""
    reward_function.before_reset(model)
    reward_function.before_reset(model_copy)


@pytest.mark.parametrize("done", [True, False])
def test_extract(reward_function, done, model):
    """Rewards are floats."""
    reward_function.before_reset(model)
    pytest.helpers.advance_to_stage(model, ecole.scip.Stage.Solving)
    reward = reward_function.extract(model, done)
    assert isinstance(reward, float)


@pytest.mark.parametrize("done", [True, False])
def test_reproducability(reward_function, done, model, model_copy):
    """Same trajectories yield same rewards."""
    reward_function.before_reset(model)
    pytest.helpers.advance_to_stage(model, ecole.scip.Stage.Solving)
    reward1 = reward_function.extract(model, done)

    reward_function.before_reset(model_copy)
    pytest.helpers.advance_to_stage(model_copy, ecole.scip.Stage.Solving)
    reward2 = reward_function.extract(model_copy, done=done)

    assert reward1 == pytest.approx(reward2, rel=1.0)


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
    pytest.helpers.advance_to_stage(model, ecole.scip.Stage.Solving)
    reward = reward_function.extract(model)
    # WARNING reward_function and formula_reward_function share underlying reference with current
    # Python implementation but the test works due to reward function reproducability.
    formula_reward_function = func_formula(reward_function)
    formula_reward_function.before_reset(model_copy)
    pytest.helpers.advance_to_stage(model_copy, ecole.scip.Stage.Solving)
    formula_reward = formula_reward_function.extract(model_copy)
    assert formula_reward == pytest.approx(reward_formula(reward), rel=1.0)


def test_cumsum(reward_function, model, model_copy):
    """Operators produce operations on rewards."""
    reward_function.before_reset(model)
    pytest.helpers.advance_to_stage(model, ecole.scip.Stage.Solving)
    reward1 = reward_function.extract(model)
    reward2 = reward_function.extract(model)
    # WARNING reward_function and cum_reward_function share underlying reference with current
    # Python implementation but the test works due to reward function reproducability.
    cum_reward_function = reward_function.cumsum()
    cum_reward_function.before_reset(model_copy)
    pytest.helpers.advance_to_stage(model_copy, ecole.scip.Stage.Solving)
    cum_reward1 = cum_reward_function.extract(model_copy)
    cum_reward2 = cum_reward_function.extract(model_copy)

    assert cum_reward1 == pytest.approx(reward1, rel=1.0)
    assert cum_reward2 == pytest.approx(reward1 + reward2, rel=1.0)


def test_primal_integral_lambda(model):
    """Tests passing a lambda function into primal integral class."""
    reward_function = ecole.reward.PrimalIntegral(bound_function=lambda x: (0, 1e3))

    reward_function.before_reset(model)
    pytest.helpers.advance_to_stage(model, ecole.scip.Stage.Solving)
    reward = reward_function.extract(model)

    assert reward >= 0


def test_dual_integral_lambda(model):
    """Tests passing a lambda function into dual integral class."""
    reward_function = ecole.reward.DualIntegral(bound_function=lambda x: (0, -1e3))

    reward_function.before_reset(model)
    pytest.helpers.advance_to_stage(model, ecole.scip.Stage.Solving)
    reward = reward_function.extract(model)

    assert reward >= 0


def test_primal_dual_integral_lambda(model):
    """Tests passing a lambda function into primal-dual integral class."""
    reward_function = ecole.reward.PrimalDualIntegral(bound_function=lambda x: (1e3, -1e3))

    reward_function.before_reset(model)
    pytest.helpers.advance_to_stage(model, ecole.scip.Stage.Solving)
    reward = reward_function.extract(model)

    assert reward >= 0
