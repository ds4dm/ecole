"""Test Ecole instance generators in Python.

The instance generators are written in python.
This file tests the instance generators with their
default set of parameters.
"""

import ecole


def test_Setcover(model):
    """ Test for Setcover instance generator. """
    instances = ecole.instances.SetCoverGenerator()
    model = next(instances)

    assert isinstance(model, ecole.core.scip.Model)


def test_CombinatorialAuction(model):
    """ Test for CombinatorialAuction instance generator. """
    instances = ecole.instances.CombinatorialAuctionGenerator()
    model = next(instances)

    assert isinstance(model, ecole.core.scip.Model)
