"""Test Ecole instance generators in Python.

The instance generators are written in python.
This file tests the instance generators with their
default set of parameters.
"""

import ecole


class ParameterGenerator:
    """ Basic parameter generator to return an empty dictionary on
        each iter.  Returning an empty dict will use the default
        parameters of each instance generator.
    """

    def __iter__(self):
        return self

    def __next__(self):
        return {}


def test_Setcover(model):
    """ Test for Setcover instance generator. """
    parameter_generator = ParameterGenerator()
    instances = ecole.instance_generators.Setcover(parameter_generator)
    model = next(instances)

    assert isinstance(model, ecole.core.scip.Model)


def test_CombinatorialAuction(model):
    """ Test for CombinatorialAuction instance generator. """
    parameter_generator = ParameterGenerator()
    instances = ecole.instance_generators.CombinatorialAuction(parameter_generator)
    next(instances)
    model = next(instances)

    assert isinstance(model, ecole.core.scip.Model)
