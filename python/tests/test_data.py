import itertools
import unittest.mock as mock

import pytest

import ecole.data


@pytest.mark.parametrize("done", (True, False))
@pytest.mark.parametrize("cste", (True, 1, "hello"))
def test_ConstantFunction(model, done, cste):
    """Always return the same constant."""
    cste_func = ecole.data.ConstantFunction(cste)
    cste_func.reset(model)
    data = cste_func.extract(model, done)
    assert data == cste


@pytest.mark.parametrize("done", (True, False))
def test_VectorFunction(model, done):
    """Dispach calls and pack the result in a list."""
    data_func1, data_func2 = mock.MagicMock(), mock.MagicMock()
    tuple_data_func = ecole.data.VectorFunction(data_func1, data_func2)

    tuple_data_func.reset(model)
    data_func1.reset.assert_called_once_with(model)
    data_func2.reset.assert_called_once_with(model)

    data_func1.extract.return_value = "something"
    data_func2.extract.return_value = "else"
    data = tuple_data_func.extract(model, done)
    assert data == ["something", "else"]


@pytest.mark.parametrize("done", (True, False))
def test_MapFunction(model, done):
    """Dispach calls and pack the result in a dict."""
    data_func1, data_func2 = mock.MagicMock(), mock.MagicMock()
    dict_data_func = ecole.data.MapFunction(name1=data_func1, name2=data_func2)

    dict_data_func.reset(model)
    data_func1.reset.assert_called_once_with(model)
    data_func2.reset.assert_called_once_with(model)

    data_func1.extract.return_value = "something"
    data_func2.extract.return_value = "else"
    data = dict_data_func.extract(model, done)
    assert data == {"name1": "something", "name2": "else"}
