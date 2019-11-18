import pytest

import ecole.observation as O
import ecole.scip


@pytest.fixture
def model(problem_file):
    return ecole.scip.Model.from_file(str(problem_file))


def test_BasicObsSpace(model):
    obs_space = O.BasicObsSpace()
    assert isinstance(obs_space, O.BasicObsSpace)
    obs = obs_space.get(model)
    assert isinstance(obs, O.BasicObs)
