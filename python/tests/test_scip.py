import pytest

import ecole.scip


def test_equality(model):
    assert model == model
    assert model != 33


def test_clone(model):
    model_copy = model.clone()
    assert model is not model_copy
    assert model != model_copy


def test_exception(model):
    with pytest.raises(ecole.scip.Exception):
        model.get_param("not_a_param")


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


@pytest.mark.parametrize("name,param_type", names_types)
def test_get_set_param(model, name, param_type):
    value = model.get_param(name)
    model.set_param(name, value)
    assert model.get_param(name) == value


should_fail = (
    # Bool in [TRUE,FALSE]
    ("branching/preferbinary", -1),
    ("branching/preferbinary", 0.1),
    ("branching/preferbinary", "toto"),
    ("branching/preferbinary", "a"),
    ("branching/preferbinary", ""),
    ("branching/preferbinary", None),
    # Int in [-1,2147483647]
    ("conflict/maxlploops", -5),
    ("conflict/maxlploops", 1e10),
    ("conflict/maxlploops", 0.1),
    ("conflict/maxlploops", "toto"),
    ("conflict/maxlploops", "a"),
    ("conflict/maxlploops", ""),
    ("conflict/maxlploops", None),
    # Long int in [-1,9223372036854775807]
    ("limits/nodes", -5),
    ("limits/nodes", 1e20),
    ("limits/nodes", 0.1),
    ("limits/nodes", "toto"),
    ("limits/nodes", "a"),
    ("limits/nodes", ""),
    ("limits/nodes", None),
    # Real in [0,1e+20]
    ("limits/time", -5),
    ("limits/time", -0.1),
    ("limits/time", 1e21),
    ("limits/time", "toto"),
    ("limits/time", "a"),
    ("limits/time", ""),
    ("limits/time", None),
    # Real in [0,1.79769313486232e+308]
    ("heuristics/undercover/maxcoversizeconss", -5),
    ("heuristics/undercover/maxcoversizeconss", -0.1),
    ("heuristics/undercover/maxcoversizeconss", "toto"),
    ("heuristics/undercover/maxcoversizeconss", "a"),
    ("heuristics/undercover/maxcoversizeconss", ""),
    ("heuristics/undercover/maxcoversizeconss", None),
    # Char in [d,s,u,v]
    # ("branching/pscost/strategy", 0), NUL character valid for SCIP ?
    ("branching/pscost/strategy", 6),
    ("branching/pscost/strategy", -5),
    ("branching/pscost/strategy", -0.1),
    ("branching/pscost/strategy", "toto"),
    ("branching/pscost/strategy", "a"),
    ("branching/pscost/strategy", ""),
    ("branching/pscost/strategy", True),
    # ("branching/pscost/strategy", False), NUL character valid for SCIP ?
    ("branching/pscost/strategy", None),
    # String, combination of 'l', 'n', 'i' characters
    ("heuristics/undercover/fixingalts", -5),
    ("heuristics/undercover/fixingalts", -0.1),
    ("heuristics/undercover/fixingalts", 6),
    # ("heuristics/undercover/fixingalts", "toto"),  valid for SCIP ?
    # ("heuristics/undercover/fixingalts", "a"),  valid for SCIP ?
    ("heuristics/undercover/fixingalts", True),
    ("heuristics/undercover/fixingalts", False),
    ("heuristics/undercover/fixingalts", None),
)


@pytest.mark.parametrize("name,value", should_fail)
def test_set_param_fail(model, name, value):
    with pytest.raises((ecole.scip.Exception, TypeError)):
        model.set_param(name, value)


should_succeed = (
    # Bool in [TRUE,FALSE]
    ("branching/preferbinary", True),
    ("branching/preferbinary", False),
    ("branching/preferbinary", 1),
    ("branching/preferbinary", 0),
    # Long int in [-1,9223372036854775807]
    ("limits/nodes", 1e18),
    ("limits/nodes", 1.0),
    ("limits/nodes", True),
    ("limits/nodes", False),
    # Real in [0,1e+20]
    ("limits/time", 0),
    ("limits/time", 0.1),
    ("limits/time", True),
    ("limits/time", False),
    # Real in [0,1.79769313486232e+308]
    ("heuristics/undercover/maxcoversizeconss", 1e200),
    # Char in [d,s,u,v]
    ("branching/pscost/strategy", "d"),
    ("branching/pscost/strategy", "s"),
    ("branching/pscost/strategy", "u"),
    ("branching/pscost/strategy", "v"),
    ("branching/pscost/strategy", 100),  # do we want to forbid this ?
    ("branching/pscost/strategy", 115),  # do we want to forbid this ?
    ("branching/pscost/strategy", 117),  # do we want to forbid this ?
    ("branching/pscost/strategy", 118),  # do we want to forbid this ?
    # String, combination of 'l', 'n', 'i' characters
    ("heuristics/undercover/fixingalts", "lni"),
    ("heuristics/undercover/fixingalts", "i"),
    ("heuristics/undercover/fixingalts", "in"),
    ("heuristics/undercover/fixingalts", ""),
    # String, any
    ("concurrent/paramsetprefix", "toto"),
    ("concurrent/paramsetprefix", "a"),
    ("concurrent/paramsetprefix", ""),
)


@pytest.mark.parametrize("name,value", should_succeed)
def test_set_param_success(model, name, value):
    model.set_param(name, value)
