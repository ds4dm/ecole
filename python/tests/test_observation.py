import ecole.observation as O


def test_BasicObsSpace(model):
    obs_space = O.BasicObsSpace()
    assert isinstance(obs_space, O.BasicObsSpace)
    obs_space.reset(model)
    obs = obs_space.get(model)
    assert isinstance(obs, O.BasicObs)
