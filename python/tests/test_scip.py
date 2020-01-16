import pytest


def test_equality(model):
    assert model == model
    assert model != 33


names_types = (
    ("branching/preferbinary", bool),  # Bool param
    ("conflict/maxlploops", int),  # Int param
    ("limits/nodes", int),  # Long int param
    ("limits/time", float),  # Real param
    ("branching/pscost/strategy", str),  # Char param
    ("concurrent/paramsetprefix", str),  # String param
)


@pytest.mark.parametrize("name,param_type", names_types)
def test_get_param(model, name, param_type):
    assert type(model.get_param(name)) == param_type


@pytest.mark.parametrize("name,param_type", names_types)
def test_set_param(model, name, param_type):
    if param_type is str:
        value = "v"  # A value accepted for the Char parameter
    else:
        value = param_type(1)  # Cast one to the required type
    model.set_param(name, value)
    assert model.get_param(name) == value
