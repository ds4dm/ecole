import unittest.mock as mock

import ecole.data


def test_VectorFunction(model):
    """Dispach calls and pack the result in a list."""
    obs_func1, obs_func2 = mock.MagicMock(), mock.MagicMock()
    tuple_obs_func = ecole.data.VectorFunction(obs_func1, obs_func2)

    tuple_obs_func.reset(model)
    obs_func1.reset.assert_called_once_with(model)
    obs_func2.reset.assert_called_once_with(model)

    obs_func1.extract.return_value = "something"
    obs_func2.extract.return_value = "else"
    obs = tuple_obs_func.extract(model, False)
    assert obs == ["something", "else"]


def test_MapFunction(model):
    """Dispach calls and pack the result in a dict."""
    obs_func1, obs_func2 = mock.MagicMock(), mock.MagicMock()
    dict_obs_func = ecole.data.MapFunction(name1=obs_func1, name2=obs_func2)

    dict_obs_func.reset(model)
    obs_func1.reset.assert_called_once_with(model)
    obs_func2.reset.assert_called_once_with(model)

    obs_func1.extract.return_value = "something"
    obs_func2.extract.return_value = "else"
    obs = dict_obs_func.extract(model, False)
    assert obs == {"name1": "something", "name2": "else"}
