"""Test Ecole observation functions in Python.

Most observation functions are written in Ecole C++ library.
This is where the logic should be tested.
Here,
  - Some tests automatically run the same assertions on all functions;
  - Other tests that observation returned form observation functions are bound to the correct types.
"""

import copy
import pickle

import numpy as np

import ecole


def pytest_generate_tests(metafunc):
    """Parametrize the `observation_function` fixture.

    Add observation functions here to have them automatically run all the tests that take
    `observation_function` as input.
    """
    if "observation_function" in metafunc.fixturenames:
        all_observation_functions = (
            ecole.observation.Nothing(),
            ecole.observation.NodeBipartite(),
            ecole.observation.MilpBipartite(),
            ecole.observation.StrongBranchingScores(True),
            ecole.observation.StrongBranchingScores(False),
            ecole.observation.Pseudocosts(),
            ecole.observation.Khalil2016(),
        )
        metafunc.parametrize("observation_function", all_observation_functions)


def advance_to_root_node(model):
    """Utility to advance a model to the root node."""
    dyn = ecole.dynamics.BranchingDynamics()
    dyn.reset_dynamics(model)
    return model


def test_default_init(observation_function):
    """Construct with default arguments."""
    type(observation_function)()


def test_before_reset(observation_function, model):
    """Successive calls to before_reset."""
    observation_function.before_reset(model)
    observation_function.before_reset(model)


def test_extract(observation_function, model):
    """Obtain observation."""
    observation_function.before_reset(model)
    advance_to_root_node(model)
    observation_function.extract(model, False)


def make_obs(obs_func, model):
    """Utility function to extract observation on root node."""
    # TODO adapt for MilpBiparite that must not be in stage solving
    obs_func.before_reset(model)
    advance_to_root_node(model)
    return obs_func.extract(model, False)


def test_observation_deepcopy(observation_function, model):
    """Deepcopy observation."""
    obs = make_obs(observation_function, model)
    copy.deepcopy(obs)


def test_observation_pickle(observation_function, model):
    """Pickle and unpickle observation."""
    obs = make_obs(observation_function, model)
    blob = pickle.dumps(obs)
    obs_copy = pickle.loads(blob)


def assert_array(arr, ndim=1, non_empty=True, dtype=np.double):
    assert isinstance(arr, np.ndarray)
    assert arr.ndim == ndim
    assert (not non_empty) or (arr.size > 0)
    assert arr.dtype == dtype


def test_Nothing_observation(model):
    """Observation of Nothing is None."""
    assert make_obs(ecole.observation.Nothing(), model) is None


def test_NodeBipartite_observation(model):
    """Observation of NodeBipartite is a type with array attributes."""
    obs = make_obs(ecole.observation.NodeBipartite(), model)
    assert isinstance(obs, ecole.observation.NodeBipartiteObs)
    assert_array(obs.column_features, ndim=2)
    assert_array(obs.row_features, ndim=2)
    assert_array(obs.edge_features.values)
    assert_array(obs.edge_features.indices, ndim=2, dtype=np.uint64)

    # Check that there are enums describing feeatures
    assert len(obs.ColumnFeatures.__members__) == obs.column_features.shape[1]
    assert len(obs.RowFeatures.__members__) == obs.row_features.shape[1]


def test_MilpBipartite_observation(model):
    """Observation of MilpBipartite is a type with array attributes."""
    obs_func = ecole.observation.MilpBipartite()
    obs_func.before_reset(model)
    obs = obs_func.extract(model, False)
    assert isinstance(obs, ecole.observation.MilpBipartiteObs)
    assert_array(obs.variable_features, ndim=2)
    assert_array(obs.constraint_features, ndim=2)
    assert_array(obs.edge_features.values)
    assert_array(obs.edge_features.indices, ndim=2, dtype=np.uint64)

    # Check that there are enums describing feeatures
    assert len(obs.VariableFeatures.__members__) == obs.variable_features.shape[1]
    assert len(obs.ConstraintFeatures.__members__) == obs.constraint_features.shape[1]


def test_StrongBranchingScores_observation(model):
    """Observation of StrongBranchingScores is a numpy array."""
    obs = make_obs(ecole.observation.StrongBranchingScores(), model)
    assert_array(obs)


def test_Pseudocosts_observation(model):
    """Observation of Pseudocosts is a numpy array."""
    obs = make_obs(ecole.observation.Pseudocosts(), model)
    assert_array(obs)


def test_Khalil2016_observatio(model):
    """Observation of Khalil2016 is a numpy matrix."""
    obs = make_obs(ecole.observation.Khalil2016(), model)
    assert_array(obs.features, ndim=2)

    # Check that there are enums describing feeatures
    assert len(obs.Features.__members__) == obs.features.shape[1]
