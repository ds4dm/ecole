import numbers

from ecole.core.data import *


def parse(something, default):
    """Recursively parse data function aggregates into their corresponding functions.

    For instance, vector of function are transformed into functions of vector, and similarily for
    maps, tuple, constants, etc.

    Parameters
    ----------
    something:
        Object to parse.
    default:
        Objet to return for when something is identified as asking for the environment specific default.

    Return
    ------
    data_func:
        A data extraction function to be used as an information, observation, or sometimes reward function.

    """
    if something == "default":
        return default
    elif something is None:
        return NoneFunction()
    elif isinstance(something, numbers.Number):
        return ConstantFunction(something)
    elif isinstance(something, (tuple, list)):
        return VectorFunction(*(parse(s, default) for s in something))
    elif isinstance(something, dict):
        return MapFunction(**{name: parse(s, default) for name, s in something.items()})
    else:
        return something
