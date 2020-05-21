from ecole.core.observation import *


class TupleFunction:
    """Pack observation functions together and return observations as tuples."""

    def __init__(self, *observation_functions):
        """Store positional observation functions."""
        self.observation_functions = observation_functions

    def reset(self, initial_state):
        """Call reset on all observation functions."""
        for obs_func in self.observation_functions:
            obs_func.reset(initial_state)

    def obtain_observation(self, state):
        """Return observation from all functions as a tuple."""
        return tuple(
            obs_func.obtain_observation(state)
            for obs_func in self.observation_functions
        )


class DictFunction:
    """Pack observation functions together and return observations as dicts."""

    def __init__(self, **observation_functions):
        """Store named observation functions."""
        self.observation_functions = observation_functions

    def reset(self, initial_state):
        """Call reset on all observation functions."""
        for obs_func in self.observation_functions.values():
            obs_func.reset(initial_state)

    def obtain_observation(self, state):
        """Return observation from all functions as a dict."""
        return {
            name: obs_func.obtain_observation(state)
            for name, obs_func in self.observation_functions.items()
        }
